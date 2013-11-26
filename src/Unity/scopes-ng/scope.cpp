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
#include <scopes/ResultItem.h>
#include <scopes/QueryCtrl.h>

namespace scopes_ng
{

using namespace unity::api;

const int AGGREGATION_TIMEOUT = 110;

class ResultCollector;

class PushEvent: public QEvent
{
public:
    static const QEvent::Type eventType;

    PushEvent(QSharedPointer<ResultCollector> collector):
        QEvent(PushEvent::eventType),
        m_collector(collector)
    {
    }

    ResultCollector* getCollector()
    {
        return m_collector.data();
    }

private:
    QSharedPointer<ResultCollector> m_collector;
};

const QEvent::Type PushEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());

class ResultCollector: public QObject
{
public:
    enum CollectionStatus { INCOMPLETE, FINISHED, CANCELLED };

    ResultCollector(QObject* receiver): QObject(nullptr), m_receiver(receiver), m_status(CollectionStatus::INCOMPLETE), m_posted(false)
    {
        m_timer.start();
    }

    // Returns bool indicating whether this resultset was already posted
    bool addResult(std::shared_ptr<scopes::ResultItem> const& result)
    {
        QMutexLocker locker(&m_mutex);
        m_results.append(result);

        return m_posted;
    }

    QList<std::shared_ptr<scopes::ResultItem>> collect(CollectionStatus& status)
    {
        QList<std::shared_ptr<scopes::ResultItem>> results;

        QMutexLocker locker(&m_mutex);
        if (m_status == CollectionStatus::INCOMPLETE) {
            // allow re-posting this collector if !resultset.finished()
            m_posted = false;
        }
        status = m_status;
        m_results.swap(results);

        return results;
    }

    void postResults(QSharedPointer<ResultCollector> collector, CollectionStatus status = CollectionStatus::INCOMPLETE)
    {
        QMutexLocker locker(&m_mutex);
        if (status != CollectionStatus::INCOMPLETE) m_status = status;
        if (m_posted || !m_receiver) return;

        // FIXME: alloc outside of the lock
        PushEvent* pushEvent = new PushEvent(collector);
        m_posted = true;

        // posting the event steals the ownership
        // also posting while the lock is held will ensure correct order
        QCoreApplication::postEvent(m_receiver, pushEvent);
    }

    void invalidate()
    {
        QMutexLocker locker(&m_mutex);
        m_receiver = nullptr;
        m_status = CollectionStatus::CANCELLED;
    }

    qint64 msecsSinceStart() const
    {
        return m_timer.elapsed();
    }

private:
    QMutex m_mutex;
    QList<std::shared_ptr<scopes::ResultItem>> m_results;
    QObject* m_receiver;
    CollectionStatus m_status;
    bool m_posted;
    // not locked
    QElapsedTimer m_timer;
};

class ResultReceiver: public scopes::ReceiverBase
{
public:
    // this will be called from non-main thread, (might even be multiple different threads)
    virtual void push(scopes::ResultItem result) override
    {
        auto res = std::make_shared<scopes::ResultItem>(std::move(result));
        bool posted = m_collector->addResult(res);
        // posting as soon as possible means we minimize delay
        if (!posted) m_collector->postResults(m_collector);
    }

    // this might be called from any thread (might be main, might be any other thread)
    virtual void finished(scopes::ReceiverBase::Reason reason) override
    {
        ResultCollector::CollectionStatus status = reason == scopes::ReceiverBase::Reason::Cancelled ?
            ResultCollector::CollectionStatus::CANCELLED : ResultCollector::CollectionStatus::FINISHED;

        m_collector->postResults(m_collector, status);
    }

    void invalidate()
    {
        m_collector->invalidate();
    }

    ResultReceiver(QObject* receiver):
        m_collector(new ResultCollector(receiver))
    {
    }

private:
    QSharedPointer<ResultCollector> m_collector;
};

Scope::Scope(QObject *parent) : QObject(parent)
    , m_formFactor("phone")
    , m_isActive(false)
    , m_searchInProgress(false)
{
    m_categories = new Categories(this);
    m_aggregatorTimer = new QTimer(this);
    m_aggregatorTimer->setSingleShot(true);
    QObject::connect(m_aggregatorTimer, &QTimer::timeout, this, &Scope::flushUpdates);
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
        ResultCollector::CollectionStatus status;
        PushEvent* pe = static_cast<PushEvent*>(ev);

        auto collector = pe->getCollector();

        auto results = collector->collect(status);
        if (status == ResultCollector::CollectionStatus::CANCELLED) {
            return true;
        }

        // qint64 inProgressMs = collector->msecsSinceStart();
        // FIXME: should we push immediately if this search is already taking a while?
        //   if we don't we're just delaying the results by another AGGREGATION_TIMEOUT ms,
        //   yet if we do, we risk getting more results right after this one

        if (m_cachedResults.empty()) {
            m_cachedResults.swap(results);
        } else {
            m_cachedResults.append(results);
        }

        if (status == ResultCollector::CollectionStatus::INCOMPLETE) {
            if (!m_aggregatorTimer->isActive()) {
                m_aggregatorTimer->start(AGGREGATION_TIMEOUT);
            }
        } else {
            // FINISHED or ERRORed collection
            m_aggregatorTimer->stop();

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

void Scope::processResultSet(QList<std::shared_ptr<scopes::ResultItem>>& result_set)
{
    if (result_set.count() == 0) return;

    // this will keep the list of categories in order
    QList<scopes::Category::SCPtr> categories;

    // split the result_set by category_id
    QMap<std::string, QList<std::shared_ptr<scopes::ResultItem>>> category_results;
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
    if (m_aggregatorTimer->isActive()) {
        m_aggregatorTimer->stop();
    }
    m_cachedResults.clear();

    // TODO: not the best idea to put the clearAll() here, can cause flicker
    m_categories->clearAll();
}

void Scope::setProxyObject(scopes::ScopeProxy const& proxy)
{
    m_proxy = proxy;
}

void Scope::setScopeId(QString const& scope_id)
{
    if (scope_id != m_scopeId) {
        m_scopeId = scope_id;
        Q_EMIT idChanged();
    }
}

QString Scope::id() const
{
    return m_scopeId;
}

QString Scope::name() const
{
    // FIXME: get from scope config
    return m_scopeId;
}

QString Scope::iconHint() const
{
    // FIXME: get from scope config
    return QString::fromStdString("");
}

QString Scope::description() const
{
    // FIXME: get from scope config
    return QString::fromStdString("");
}

QString Scope::searchHint() const
{
    // FIXME: get from scope config
    return QString::fromStdString("Search");
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
    // FIXME: get from scope config
    return QString::fromStdString("");
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
        if (m_proxy) {
            scopes::VariantMap vm;
            m_lastReceiver.reset(new ResultReceiver(this));
            m_lastQuery = m_proxy->create_query(search_query.toStdString(), vm, m_lastReceiver);
        }
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
