/*
 * Copyright (C) 2014 Canonical, Ltd.
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

// local
#include "utils.h"

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
        m_previewData.insert(QString::fromStdString(key), scopeVariantToQVariant(value));

        return m_posted;
    }

    bool setColumnLayoutList(scopes::ColumnLayoutList const& columns)
    {
        QMutexLocker locker(&m_mutex);
        m_columnLayouts = columns;

        return m_posted;
    }

    Status collect(scopes::ColumnLayoutList& out_columns, scopes::PreviewWidgetList& out_widgets, QHash<QString, QVariant>& out_data)
    {
        Status status;

        QMutexLocker locker(&m_mutex);
        if (m_status == Status::INCOMPLETE) {
            // allow re-posting this collector if !resultset.finished()
            m_posted = false;
        }
        status = m_status;
        m_columnLayouts.swap(out_columns);
        m_widgets.swap(out_widgets);
        m_previewData.swap(out_data);

        return status;
    }

private:
    scopes::ColumnLayoutList m_columnLayouts;
    scopes::PreviewWidgetList m_widgets;
    QHash<QString, QVariant> m_previewData;
};

class ActivationCollector: public CollectorBase
{
public:
    ActivationCollector(std::shared_ptr<scopes::Result> const& result): CollectorBase(), m_result(result)
    {
    }

    // Returns bool indicating whether this resultset was already posted
    void setResponse(scopes::ActivationResponse const& response)
    {
        QMutexLocker locker(&m_mutex);
        m_response.reset(new scopes::ActivationResponse(response));
    }

    Status collect(std::shared_ptr<scopes::ActivationResponse>& out_response, std::shared_ptr<scopes::Result>& out_result)
    {
        Status status;

        QMutexLocker locker(&m_mutex);
        if (m_status == Status::INCOMPLETE) {
            // allow re-posting this collector if !resultset.finished()
            m_posted = false;
        }
        status = m_status;
        out_response = m_response;
        out_result = m_result;

        return status;
    }

private:
    std::shared_ptr<scopes::ActivationResponse> m_response;
    std::shared_ptr<scopes::Result> m_result;
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


qint64 PushEvent::msecsSinceStart() const
{
    return m_collector->msecsSinceStart();
}

CollectorBase::Status PushEvent::collectSearchResults(QList<std::shared_ptr<scopes::CategorisedResult>>& out_results)
{
    auto collector = std::dynamic_pointer_cast<ResultCollector>(m_collector);
    return collector->collect(out_results);
}

CollectorBase::Status PushEvent::collectPreviewData(scopes::ColumnLayoutList& out_columns, scopes::PreviewWidgetList& out_widgets, QHash<QString, QVariant>& out_data)
{
    auto collector = std::dynamic_pointer_cast<PreviewDataCollector>(m_collector);
    return collector->collect(out_columns, out_widgets, out_data);
}

CollectorBase::Status PushEvent::collectActivationResponse(std::shared_ptr<scopes::ActivationResponse>& out_response, std::shared_ptr<scopes::Result>& out_result)
{
    auto collector = std::dynamic_pointer_cast<ActivationCollector>(m_collector);
    return collector->collect(out_response, out_result);
}

ScopeDataReceiverBase::ScopeDataReceiverBase(QObject* receiver, PushEvent::Type push_type, 
                                             std::shared_ptr<CollectorBase> const& collector):
    m_receiver(receiver), m_eventType(push_type), m_collector(collector)
{
}

void ScopeDataReceiverBase::postCollectedResults(CollectorBase::Status status)
{
    if (m_collector->submit(status)) {
        QScopedPointer<PushEvent> pushEvent(new PushEvent(m_eventType, m_collector));
        QMutexLocker locker(&m_mutex);
        // posting the event steals the ownership
        if (m_receiver == nullptr) return;
        QCoreApplication::postEvent(m_receiver, pushEvent.take());
    }
}

void ScopeDataReceiverBase::invalidate()
{
    m_collector->invalidate();
    QMutexLocker locker(&m_mutex);
    m_receiver = nullptr;
}

SearchResultReceiver::SearchResultReceiver(QObject* receiver):
    ScopeDataReceiverBase(receiver, PushEvent::SEARCH, std::shared_ptr<CollectorBase>(new ResultCollector))
{
    m_collector = collectorAs<ResultCollector>();
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

PreviewDataReceiver::PreviewDataReceiver(QObject* receiver):
    ScopeDataReceiverBase(receiver, PushEvent::PREVIEW, std::shared_ptr<CollectorBase>(new PreviewDataCollector))
{
    m_collector = collectorAs<PreviewDataCollector>();
}

// this will be called from non-main thread, (might even be multiple different threads)
void PreviewDataReceiver::push(unity::scopes::ColumnLayoutList const& layouts)
{
    bool posted = m_collector->setColumnLayoutList(layouts);
    if (!posted) {
        postCollectedResults();
    }
}

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

void ActivationReceiver::activated(scopes::ActivationResponse const& response)
{
    m_collector->setResponse(response);
}

void ActivationReceiver::finished(scopes::ListenerBase::Reason reason, std::string const& error_msg)
{
    Q_UNUSED(error_msg);
    CollectorBase::Status status = reason == scopes::ListenerBase::Reason::Cancelled ?
        CollectorBase::Status::CANCELLED : CollectorBase::Status::FINISHED;

    postCollectedResults(status);
}

ActivationReceiver::ActivationReceiver(QObject* receiver, std::shared_ptr<scopes::Result> const& result):
    ScopeDataReceiverBase(receiver, PushEvent::ACTIVATION, std::shared_ptr<CollectorBase>(new ActivationCollector(result)))
{
    m_collector = collectorAs<ActivationCollector>();
}

} // namespace scopes_ng
