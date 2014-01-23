/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Michal Hruby <michal.hruby@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// Self
#include "collectors.h"

// Qt
#include <QDebug>
#include <QEvent>
#include <QMutex>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QScopedPointer>

namespace scopes_ng
{

using namespace unity;

const QEvent::Type PushEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());

CollectorBase::CollectorBase(): m_status(Status::INCOMPLETE), m_posted(false)
{
    m_timer.start();
}

CollectorBase::~CollectorBase()
{
}

/* Returns bool indicating whether the collector should be sent to the UI thread */
bool CollectorBase::submit(Status status)
{
    QMutexLocker locker(&m_mutex);
    if (m_status == Status::INCOMPLETE) m_status = status;
    if (m_posted) return false;

    m_posted = true;

    return true;
}

void CollectorBase::invalidate()
{
    QMutexLocker locker(&m_mutex);
    m_status = Status::CANCELLED;
}

qint64 CollectorBase::msecsSinceStart() const
{
    return m_timer.elapsed();
}

class ResultCollector: public CollectorBase
{
public:
    ResultCollector(): CollectorBase()
    {
    }

    // Returns bool indicating whether this resultset was already posted
    bool addResult(std::shared_ptr<scopes::CategorisedResult> const& result)
    {
        QMutexLocker locker(&m_mutex);
        m_results.append(result);

        return m_posted;
    }

    Status collect(QList<std::shared_ptr<scopes::CategorisedResult>>& out_results)
    {
        Status status;

        QMutexLocker locker(&m_mutex);
        if (m_status == Status::INCOMPLETE) {
            // allow re-posting this collector if !resultset.finished()
            m_posted = false;
        }
        status = m_status;
        m_results.swap(out_results);

        return status;
    }

private:
    QList<std::shared_ptr<scopes::CategorisedResult>> m_results;
};

class PreviewDataCollector: public CollectorBase
{
public:
    PreviewDataCollector(): CollectorBase()
    {
    }

    // Returns bool indicating whether this resultset was already posted
    bool addWidgets(scopes::PreviewWidgetList const& widgets)
    {
        QMutexLocker locker(&m_mutex);
        m_widgets.insert(m_widgets.end(), widgets.begin(), widgets.end());

        return m_posted;
    }

    bool addData(std::string const& key, scopes::Variant const& value)
    {
        QMutexLocker locker(&m_mutex);
        m_previewData[QString::fromStdString(key)] = value;

        return m_posted;
    }

    Status collect(scopes::PreviewWidgetList& out_widgets, QHash<QString, scopes::Variant>& out_data)
    {
        Status status;

        QMutexLocker locker(&m_mutex);
        if (m_status == Status::INCOMPLETE) {
            // allow re-posting this collector if !resultset.finished()
            m_posted = false;
        }
        status = m_status;
        m_widgets.swap(out_widgets);
        m_previewData.swap(out_data);

        return status;
    }

private:
    scopes::PreviewWidgetList m_widgets;
    QHash<QString, scopes::Variant> m_previewData;
};

PushEvent::PushEvent(Type event_type, std::shared_ptr<CollectorBase> collector):
    QEvent(PushEvent::eventType),
    m_eventType(event_type),
    m_collector(collector)
{
}

PushEvent::Type PushEvent::type()
{
    return m_eventType;
}

CollectorBase::Status PushEvent::collectResults(QList<std::shared_ptr<scopes::CategorisedResult>>& out_results)
{
    auto collector = std::dynamic_pointer_cast<ResultCollector>(m_collector);
    return collector->collect(out_results);
}

CollectorBase::Status PushEvent::collectPreviewData(scopes::PreviewWidgetList& out_widgets, QHash<QString, scopes::Variant>& out_data)
{
    auto collector = std::dynamic_pointer_cast<PreviewDataCollector>(m_collector);
    return collector->collect(out_widgets, out_data);
}

// this will be called from non-main thread, (might even be multiple different threads)
void SearchResultReceiver::push(scopes::CategorisedResult result)
{
    auto res = std::make_shared<scopes::CategorisedResult>(std::move(result));
    bool posted = m_collector->addResult(res);
    // posting as soon as possible means we minimize delay
    if (!posted) {
        postCollectedResults();
    }
}

// this might be called from any thread (might be main, might be any other thread)
void SearchResultReceiver::finished(scopes::ListenerBase::Reason reason, std::string const& error_msg)
{
    Q_UNUSED(error_msg);
    CollectorBase::Status status = reason == scopes::ListenerBase::Reason::Cancelled ?
        CollectorBase::Status::CANCELLED : CollectorBase::Status::FINISHED;

    postCollectedResults(status);
}

void SearchResultReceiver::invalidate()
{
    m_collector->invalidate();
    QMutexLocker locker(&m_mutex);
    m_receiver = nullptr;
}

SearchResultReceiver::SearchResultReceiver(QObject* receiver):
    m_collector(new ResultCollector), m_receiver(receiver)
{
}

void SearchResultReceiver::postCollectedResults(CollectorBase::Status status)
{
    if (m_collector->submit(status)) {
        QScopedPointer<PushEvent> pushEvent(new PushEvent(PushEvent::SEARCH, m_collector));
        QMutexLocker locker(&m_mutex);
        // posting the event steals the ownership
        if (m_receiver == nullptr) return;
        QCoreApplication::postEvent(m_receiver, pushEvent.take());
    }
}

// this will be called from non-main thread, (might even be multiple different threads)
void PreviewDataReceiver::push(scopes::PreviewWidgetList const& widgets)
{
    bool posted = m_collector->addWidgets(widgets);
    if (!posted) {
        postCollectedResults();
    }
}

void PreviewDataReceiver::push(std::string const& key, scopes::Variant const& value)
{
    bool posted = m_collector->addData(key, value);
    if (!posted) {
        postCollectedResults();
    }
}

// this might be called from any thread (might be main, might be any other thread)
void PreviewDataReceiver::finished(scopes::ListenerBase::Reason reason, std::string const& error_msg)
{
    Q_UNUSED(error_msg);
    CollectorBase::Status status = reason == scopes::ListenerBase::Reason::Cancelled ?
        CollectorBase::Status::CANCELLED : CollectorBase::Status::FINISHED;

    postCollectedResults(status);
}

void PreviewDataReceiver::invalidate()
{
    m_collector->invalidate();
    QMutexLocker locker(&m_mutex);
    m_receiver = nullptr;
}

PreviewDataReceiver::PreviewDataReceiver(QObject* receiver):
    m_collector(new PreviewDataCollector), m_receiver(receiver)
{
}

void PreviewDataReceiver::postCollectedResults(CollectorBase::Status status)
{
    if (m_collector->submit(status)) {
        QScopedPointer<PushEvent> pushEvent(new PushEvent(PushEvent::PREVIEW, m_collector));
        QMutexLocker locker(&m_mutex);
        // posting the event steals the ownership
        if (m_receiver == nullptr) return;
        QCoreApplication::postEvent(m_receiver, pushEvent.take());
    }
}

} // namespace scopes_ng
