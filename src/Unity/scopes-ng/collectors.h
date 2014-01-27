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

// Qt
#include <QDebug>
#include <QEvent>
#include <QMutex>
#include <QElapsedTimer>

#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/SearchListener.h>
#include <unity/scopes/PreviewListener.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/QueryCtrl.h>
#include <unity/scopes/PreviewWidget.h>

namespace scopes_ng
{

class ResultCollector;
class PreviewDataCollector;

class CollectorBase
{
public:
    enum Status { INCOMPLETE, FINISHED, CANCELLED };

    CollectorBase();
    virtual ~CollectorBase();

    bool submit(Status status = Status::INCOMPLETE);
    void invalidate();
    qint64 msecsSinceStart() const;

protected:
    QMutex m_mutex;
    Status m_status;
    bool m_posted;

private:
    // not locked
    QElapsedTimer m_timer;
};

class PushEvent: public QEvent
{
public:
    static const QEvent::Type eventType;

    enum Type { SEARCH, PREVIEW };

    PushEvent(Type event_type, std::shared_ptr<CollectorBase> collector);
    Type type();

    CollectorBase::Status collectResults(QList<std::shared_ptr<unity::scopes::CategorisedResult>>& out_results);
    CollectorBase::Status collectPreviewData(unity::scopes::PreviewWidgetList& out_widgets, QHash<QString, QVariant>& out_data);

private:
    Type m_eventType;
    std::shared_ptr<CollectorBase> m_collector;
};

class SearchResultReceiver: public unity::scopes::SearchListener
{
public:
    virtual void push(unity::scopes::CategorisedResult result) override;
    virtual void finished(unity::scopes::ListenerBase::Reason reason, std::string const& error_msg) override;

    void invalidate();

    SearchResultReceiver(QObject* receiver);

private:
    void postCollectedResults(CollectorBase::Status status = CollectorBase::Status::INCOMPLETE);

    std::shared_ptr<ResultCollector> m_collector;
    QMutex m_mutex;
    QObject* m_receiver;
};

class PreviewDataReceiver: public unity::scopes::PreviewListener
{
public:
    virtual void push(unity::scopes::PreviewWidgetList const& widgets) override;
    virtual void push(std::string const& key, unity::scopes::Variant const& value) override;
    virtual void finished(unity::scopes::ListenerBase::Reason reason, std::string const& error_msg) override;

    void invalidate();

    PreviewDataReceiver(QObject* receiver);

private:
    void postCollectedResults(CollectorBase::Status status = CollectorBase::Status::INCOMPLETE);

    std::shared_ptr<PreviewDataCollector> m_collector;
    QMutex m_mutex;
    QObject* m_receiver;
};

} // namespace scopes_ng
