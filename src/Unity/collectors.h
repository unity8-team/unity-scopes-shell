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

#ifndef NG_COLLECTORS_H
#define NG_COLLECTORS_H

// Qt
#include <QDebug>
#include <QEvent>
#include <QMutex>
#include <QElapsedTimer>

#include <unity/scopes/ActivationListenerBase.h>
#include <unity/scopes/ActivationResponse.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/FilterBase.h>
#include <unity/scopes/FilterState.h>
#include <unity/scopes/OptionSelectorFilter.h>
#include <unity/scopes/PreviewListenerBase.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/QueryCtrl.h>
#include <unity/scopes/SearchListenerBase.h>

namespace scopes_ng
{

class SearchDataCollector;
class PreviewDataCollector;
class ActivationCollector;

class CollectorBase
{
public:
    enum Status { UNKNOWN, INCOMPLETE, FINISHED, CANCELLED, NO_INTERNET, NO_LOCATION_DATA };

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

    PushEvent(Type event_type, const std::shared_ptr<CollectorBase>& collector);
    Type type();

    CollectorBase::Status collectSearchResults(QList<std::shared_ptr<unity::scopes::CategorisedResult>>& out_results, unity::scopes::Department::SCPtr&
            out_rootDepartment, unity::scopes::OptionSelectorFilter::SCPtr& out_sortOrder, QList<unity::scopes::FilterBase::SCPtr>& out_filters);
    CollectorBase::Status collectPreviewData(unity::scopes::ColumnLayoutList& out_columns, unity::scopes::PreviewWidgetList& out_widgets, QHash<QString, QVariant>& out_data);
    CollectorBase::Status collectActivationResponse(std::shared_ptr<unity::scopes::ActivationResponse>& out_response, std::shared_ptr<unity::scopes::Result>&
            out_result, QString& categoryId);

    qint64 msecsSinceStart() const;

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

class SearchResultReceiver: public unity::scopes::SearchListenerBase, public ScopeDataReceiverBase
{
public:
    virtual void push(unity::scopes::CategorisedResult result) override;
    virtual void push(unity::scopes::Department::SCPtr const& department) override;
    virtual void push(unity::scopes::Filters const& filters, unity::scopes::FilterState const& state) override;
    virtual void finished(unity::scopes::CompletionDetails const& details) override;

    SearchResultReceiver(QObject* receiver);

private:
    std::shared_ptr<SearchDataCollector> m_collector;
};

class PreviewDataReceiver: public unity::scopes::PreviewListenerBase, public ScopeDataReceiverBase
{
public:
    virtual void push(unity::scopes::ColumnLayoutList const& layouts) override;
    virtual void push(unity::scopes::PreviewWidgetList const& widgets) override;
    virtual void push(std::string const& key, unity::scopes::Variant const& value) override;
    virtual void finished(unity::scopes::CompletionDetails const& details) override;

    PreviewDataReceiver(QObject* receiver);

private:
    std::shared_ptr<PreviewDataCollector> m_collector;
};

class ActivationReceiver: public unity::scopes::ActivationListenerBase, public ScopeDataReceiverBase
{
public:
    virtual void activated(unity::scopes::ActivationResponse const&) override;
    virtual void finished(unity::scopes::CompletionDetails const& details) override;

    ActivationReceiver(QObject* receiver, std::shared_ptr<unity::scopes::Result> const& result, QString const& categoryId = "");

private:
    std::shared_ptr<ActivationCollector> m_collector;
    QString m_categoryId;
};

} // namespace scopes_ng

#endif // NG_COLLECTORS_H
