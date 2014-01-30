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

// Qt
#include <QDebug>
#include <QEvent>
#include <QMutex>
#include <QElapsedTimer>

#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/ActivationListener.h>
#include <unity/scopes/SearchListener.h>
#include <unity/scopes/PreviewListener.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/QueryCtrl.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/ActivationResponse.h>

namespace scopes_ng
{

class ResultCollector;
class PreviewDataCollector;
class ActivationCollector;

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

    enum Type { SEARCH, PREVIEW, ACTIVATION };

    PushEvent(Type event_type, std::shared_ptr<CollectorBase> collector);
    Type type();

    CollectorBase::Status collectSearchResults(QList<std::shared_ptr<unity::scopes::CategorisedResult>>& out_results);
    CollectorBase::Status collectPreviewData(unity::scopes::PreviewWidgetList& out_widgets, QHash<QString, QVariant>& out_data);
    CollectorBase::Status collectActivationResponse(std::shared_ptr<unity::scopes::ActivationResponse>& out_response, std::shared_ptr<unity::scopes::Result>& out_result);

private:
    Type m_eventType;
    std::shared_ptr<CollectorBase> m_collector;
};

class ScopeDataReceiverBase
{
public:
    ScopeDataReceiverBase(QObject* receiver, PushEvent::Type push_type, std::shared_ptr<CollectorBase> const& collector);

    void invalidate();
    template<typename T> std::shared_ptr<T> collectorAs() { return std::dynamic_pointer_cast<T>(m_collector); }
protected:
    void postCollectedResults(CollectorBase::Status status = CollectorBase::Status::INCOMPLETE);
private:
    QMutex m_mutex;
    QObject* m_receiver;
    PushEvent::Type m_eventType;
    std::shared_ptr<CollectorBase> m_collector;
};

class SearchResultReceiver: public unity::scopes::SearchListener, public ScopeDataReceiverBase
{
public:
    virtual void push(unity::scopes::CategorisedResult result) override;
    virtual void finished(unity::scopes::ListenerBase::Reason reason, std::string const& error_msg) override;

    SearchResultReceiver(QObject* receiver);

private:
    std::shared_ptr<ResultCollector> m_collector;
};

class PreviewDataReceiver: public unity::scopes::PreviewListener, public ScopeDataReceiverBase
{
public:
    virtual void push(unity::scopes::ColumnLayoutList const& layouts) override;
    virtual void push(unity::scopes::PreviewWidgetList const& widgets) override;
    virtual void push(std::string const& key, unity::scopes::Variant const& value) override;
    virtual void finished(unity::scopes::ListenerBase::Reason reason, std::string const& error_msg) override;

    PreviewDataReceiver(QObject* receiver);

private:
    std::shared_ptr<PreviewDataCollector> m_collector;
};

class ActivationReceiver: public unity::scopes::ActivationListener, public ScopeDataReceiverBase
{
public:
    virtual void activation_response(unity::scopes::ActivationResponse const&) override;
    virtual void finished(unity::scopes::ListenerBase::Reason reason, std::string const& error_msg) override;

    ActivationReceiver(QObject* receiver, std::shared_ptr<unity::scopes::Result> const& result);

private:
    std::shared_ptr<ActivationCollector> m_collector;
};

} // namespace scopes_ng
