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
#include "scope.h"

// local
#include "categories.h"
//#include "preview.h"

// Qt
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QtGui/QDesktopServices>
#include <QQmlEngine>
#include <QEvent>
#include <QMutex>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QElapsedTimer>

#include <libintl.h>
#include <glib.h>

#include <scopes/ReceiverBase.h>
#include <scopes/CategorisedResult.h>
#include <scopes/QueryCtrl.h>

namespace scopes_ng
{

using namespace unity::api;

const int AGGREGATION_TIMEOUT = 110;

class ResultCollector: public QObject
{
public:
    enum CollectionStatus { INCOMPLETE, FINISHED, CANCELLED };

    ResultCollector(): QObject(nullptr), m_status(CollectionStatus::INCOMPLETE), m_posted(false)
    {
        m_timer.start();
    }

    // Returns bool indicating whether this resultset was already posted
    bool addResult(std::shared_ptr<scopes::CategorisedResult> const& result)
    {
        QMutexLocker locker(&m_mutex);
        m_results.append(result);

        return m_posted;
    }

    QList<std::shared_ptr<scopes::CategorisedResult>> collect(CollectionStatus& out_status)
    {
        QList<std::shared_ptr<scopes::CategorisedResult>> results;

        QMutexLocker locker(&m_mutex);
        if (m_status == CollectionStatus::INCOMPLETE) {
            // allow re-posting this collector if !resultset.finished()
            m_posted = false;
        }
        out_status = m_status;
        m_results.swap(results);

        return results;
    }

    /* Returns bool indicating whether the collector should be sent to the UI thread */
    bool submit(CollectionStatus status = CollectionStatus::INCOMPLETE)
    {
        QMutexLocker locker(&m_mutex);
        if (m_status == CollectionStatus::INCOMPLETE) m_status = status;
        if (m_posted) return false;

        m_posted = true;

        return true;
    }

    void invalidate()
    {
        QMutexLocker locker(&m_mutex);
        m_status = CollectionStatus::CANCELLED;
    }

    qint64 msecsSinceStart() const
    {
        return m_timer.elapsed();
    }

private:
    QMutex m_mutex;
    QList<std::shared_ptr<scopes::CategorisedResult>> m_results;
    CollectionStatus m_status;
    bool m_posted;
    // not locked
    QElapsedTimer m_timer;
};

class PushEvent: public QEvent
{
public:
    static const QEvent::Type eventType;

    PushEvent(QSharedPointer<ResultCollector> collector):
        QEvent(PushEvent::eventType),
        m_collector(collector)
    {
    }

    QList<std::shared_ptr<scopes::CategorisedResult>> collect(ResultCollector::CollectionStatus& out_status)
    {
        return m_collector->collect(out_status);
    }

private:
    QSharedPointer<ResultCollector> m_collector;
};

const QEvent::Type PushEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());

class ResultReceiver: public scopes::ReceiverBase
{
public:
    // this will be called from non-main thread, (might even be multiple different threads)
    virtual void push(scopes::CategorisedResult result) override
    {
        auto res = std::make_shared<scopes::CategorisedResult>(std::move(result));
        bool posted = m_collector->addResult(res);
        // posting as soon as possible means we minimize delay
        if (!posted) {
            postCollectedResults();
        }
    }

    // this might be called from any thread (might be main, might be any other thread)
    virtual void finished(scopes::ReceiverBase::Reason reason) override
    {
        ResultCollector::CollectionStatus status = reason == scopes::ReceiverBase::Reason::Cancelled ?
            ResultCollector::CollectionStatus::CANCELLED : ResultCollector::CollectionStatus::FINISHED;

        postCollectedResults(status);
    }

    void invalidate()
    {
        m_collector->invalidate();
        QMutexLocker locker(&m_mutex);
        m_receiver = nullptr;
    }

    ResultReceiver(QObject* receiver):
        m_collector(new ResultCollector), m_receiver(receiver)
    {
    }

private:
    void postCollectedResults(ResultCollector::CollectionStatus status = ResultCollector::CollectionStatus::INCOMPLETE)
    {
        if (m_collector->submit(status)) {
            auto pushEvent = new PushEvent(m_collector);
            QMutexLocker locker(&m_mutex);
            // posting the event steals the ownership
            if (m_receiver == nullptr) return;
            QCoreApplication::postEvent(m_receiver, pushEvent);
        }
    }

    QSharedPointer<ResultCollector> m_collector;
    QMutex m_mutex;
    QObject* m_receiver;
};

Scope::Scope(QObject *parent) : QObject(parent)
    , m_formFactor("phone")
    , m_isActive(false)
    , m_searchInProgress(false)
{
    m_categories = new Categories(this);
    m_aggregatorTimer.setSingleShot(true);
    QObject::connect(&m_aggregatorTimer, &QTimer::timeout, this, &Scope::flushUpdates);
}

Scope::~Scope()
{
    if (m_lastReceiver) {
        std::dynamic_pointer_cast<ResultReceiver>(m_lastReceiver)->invalidate();
    }
}

bool Scope::event(QEvent* ev)
{
    if (ev->type() == PushEvent::eventType) {
        PushEvent* pushEvent = static_cast<PushEvent*>(ev);

        ResultCollector::CollectionStatus status;
        auto results = pushEvent->collect(status);
        if (status == ResultCollector::CollectionStatus::CANCELLED) {
            return true;
        }

        // qint64 inProgressMs = collector->msecsSinceStart();
        // FIXME: should we push immediately if this search is already taking a while?
        //   if we don't, we're just delaying the results by another AGGREGATION_TIMEOUT ms,
        //   yet if we do, we risk getting more results right after this one

        if (m_cachedResults.empty()) {
            m_cachedResults.swap(results);
        } else {
            m_cachedResults.append(results);
        }

        if (status == ResultCollector::CollectionStatus::INCOMPLETE) {
            if (!m_aggregatorTimer.isActive()) {
                m_aggregatorTimer.start(AGGREGATION_TIMEOUT);
            }
        } else {
            // FINISHED or ERRORed collection
            m_aggregatorTimer.stop();

            flushUpdates();

            if (m_searchInProgress) {
                m_searchInProgress = false;
                Q_EMIT searchInProgressChanged();
            }
        }

        return true;
    }
    return QObject::event(ev);
}

void Scope::flushUpdates()
{
    processResultSet(m_cachedResults); // clears the result list
}

void Scope::processResultSet(QList<std::shared_ptr<scopes::CategorisedResult>>& result_set)
{
    if (result_set.count() == 0) return;

    // this will keep the list of categories in order
    QList<scopes::Category::SCPtr> categories;

    // split the result_set by category_id
    QMap<std::string, QList<std::shared_ptr<scopes::CategorisedResult>>> category_results;
    while (!result_set.empty()) {
        auto result = result_set.takeFirst();
        if (!category_results.contains(result->category()->id())) {
            categories.append(result->category());
        }
        category_results[result->category()->id()].append(std::move(result));
    }

    Q_FOREACH(scopes::Category::SCPtr const& category, categories) {
        ResultsModel* category_model = m_categories->lookupCategory(category->id());
        if (category_model == nullptr) {
            category_model = new ResultsModel(m_categories);
            category_model->setCategoryId(QString::fromStdString(category->id()));
            category_model->addResults(category_results[category->id()]);
            m_categories->registerCategory(category, category_model);
        } else {
            // FIXME: only update when we know it's necessary
            m_categories->registerCategory(category, nullptr);
            category_model->addResults(category_results[category->id()]);
            m_categories->updateResultCount(category_model);
        }
    }
}

void Scope::invalidateLastSearch()
{
    if (m_lastReceiver) {
        std::dynamic_pointer_cast<ResultReceiver>(m_lastReceiver)->invalidate();
        m_lastReceiver.reset();
    }
    if (m_lastQuery) {
        m_lastQuery->cancel();
        m_lastQuery.reset();
    }
    if (m_aggregatorTimer.isActive()) {
        m_aggregatorTimer.stop();
    }
    m_cachedResults.clear();

    // TODO: not the best idea to put the clearAll() here, can cause flicker
    m_categories->clearAll();
}

void Scope::dispatchSearch()
{
    /* There are a few objects associated with searches:
     * 1) ResultReceiver    2) ResultCollector    3) PushEvent
     *
     * ResultReceiver is associated with the search and has methods that get called
     * by the scopes framework when results / categories / annotations are received.
     * Since the notification methods (push(...)) of ResultReceiver are called
     * from different thread(s), it uses ResultCollector to collect multiple results
     * in a thread-safe manner.
     * Once a couple of results are collected, the collector is sent via a PushEvent
     * to the UI thread, where it is processed. When the results are collected by the UI thread,
     * the collector continues to collect more results, and uses another PushEvent to post them.
     *
     * If a new query is submitted the previous one is marked as cancelled by invoking
     * ResultReceiver::invalidate() and any PushEvent that is waiting to be processed
     * will be discarded as the collector will also be marked as invalid.
     * The new query will have new instances of ResultReceiver and ResultCollector.
     */

    if (m_proxy) {
        scopes::VariantMap vm;
        m_lastReceiver.reset(new ResultReceiver(this));
        m_lastQuery = m_proxy->create_query(m_searchQuery.toStdString(), vm, m_lastReceiver);
    }
}

void Scope::setScopeData(scopes::ScopeMetadata const& data)
{
    m_scopeMetadata = std::make_shared<scopes::ScopeMetadata>(data);
    m_proxy = data.proxy();
}

QString Scope::id() const
{
    return QString::fromStdString(m_scopeMetadata ? m_scopeMetadata->scope_name() : "");
}

QString Scope::name() const
{
    return QString::fromStdString(m_scopeMetadata ? m_scopeMetadata->display_name() : "");
}

QString Scope::iconHint() const
{
    std::string icon;
    try {
        if (m_scopeMetadata) {
            icon = m_scopeMetadata->icon();
        }
    } catch (...) {
        // throws if the value isn't set, safe to ignore
    }

    return QString::fromStdString(icon);
}

QString Scope::description() const
{
    return QString::fromStdString(m_scopeMetadata ? m_scopeMetadata->description() : "");
}

QString Scope::searchHint() const
{
    return QString::fromStdString(m_scopeMetadata ? m_scopeMetadata->search_hint() : "");
}

bool Scope::searchInProgress() const
{
    return m_searchInProgress;
}

bool Scope::visible() const
{
    // FIXME: get from scope config
    return true;
}

QString Scope::shortcut() const
{
    std::string hotkey;
    try {
        if (m_scopeMetadata) {
            hotkey = m_scopeMetadata->hot_key();
        }
    } catch (...) {
        // throws if the value isn't set, safe to ignore
    }

    return QString::fromStdString(hotkey);
}

bool Scope::connected() const
{
    // FIXME: get from scope proxy?
    return false;
}

Categories* Scope::categories() const
{
    return m_categories;
}

/*
Filters* Scope::filters() const
{
    return m_filters.get();
}
*/

QString Scope::searchQuery() const
{
    return m_searchQuery;
}

QString Scope::noResultsHint() const
{
    return m_noResultsHint;
}

QString Scope::formFactor() const
{
    return m_formFactor;
}

bool Scope::isActive() const
{
    return m_isActive;
}

void Scope::setSearchQuery(const QString& search_query)
{
    /* Checking for m_searchQuery.isNull() which returns true only when the string
       has never been set is necessary because when search_query is the empty
       string ("") and m_searchQuery is the null string,
       search_query != m_searchQuery is still true.
    */

    if (m_searchQuery.isNull() || search_query != m_searchQuery) {
        m_searchQuery = search_query;

        invalidateLastSearch();
        // FIXME: use a timeout
        dispatchSearch();

        Q_EMIT searchQueryChanged();
        if (!m_searchInProgress) {
            m_searchInProgress = true;
            Q_EMIT searchInProgressChanged();
        }
    }
}

void Scope::setNoResultsHint(const QString& hint) {
    if (hint != m_noResultsHint) {
        m_noResultsHint = hint;
        Q_EMIT noResultsHintChanged();
    }
}

void Scope::setFormFactor(const QString& form_factor) {
    if (form_factor != m_formFactor) {
        m_formFactor = form_factor;
        // FIXME: force new search
        Q_EMIT formFactorChanged();
    }
}

void Scope::setActive(const bool active) {
    if (active != m_isActive) {
        m_isActive = active;
        Q_EMIT isActiveChanged(m_isActive);
    }
}

// FIXME: Change to use row index.
void Scope::activate(const QVariant &uri, const QVariant &icon_hint, const QVariant &category,
                     const QVariant &result_type, const QVariant &mimetype, const QVariant &title,
                     const QVariant &comment, const QVariant &dnd_uri, const QVariant &metadata)
{
}

// FIXME: Change to use row index.
void Scope::preview(const QVariant &uri, const QVariant &icon_hint, const QVariant &category,
             const QVariant &result_type, const QVariant &mimetype, const QVariant &title,
             const QVariant &comment, const QVariant &dnd_uri, const QVariant &metadata)
{
    // FIXME: handle overridden results
}

void Scope::cancelActivation()
{
}

} // namespace scopes_ng
