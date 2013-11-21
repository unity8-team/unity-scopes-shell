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

#include <libintl.h>
#include <glib.h>

#include <scopes/ReceiverBase.h>
#include <scopes/ResultItem.h>
#include <scopes/QueryCtrl.h>

namespace scopes_ng
{

using namespace unity::api;

class ResultCollector: public QObject
{
public:
    ResultCollector(): QObject(nullptr), m_finished(false), m_posted(false)
    {
    }

    /* Returns true if result was added, false if the event no longer accepts
     * more results */
    void addResult(std::shared_ptr<scopes::ResultItem> const& result)
    {
        QMutexLocker locker(&m_mutex);
        m_results.append(result);
    }

    QList<std::shared_ptr<scopes::ResultItem>> getResults()
    {
        QMutexLocker locker(&m_mutex);
        m_posted = false;

        QList<std::shared_ptr<scopes::ResultItem>> results;
        m_results.swap(results);

        return results;
    }

    bool finished()
    {
        QMutexLocker locker(&m_mutex);
        return m_finished;
    }

    void setFinished(bool finished)
    {
        QMutexLocker locker(&m_mutex);
        m_finished = finished;
    }

    // Returns whether this resultset was already posted, note that call
    // to getResults() resets the flag and allows to post the resultset again
    bool posted()
    {
        QMutexLocker locker(&m_mutex);
        return m_posted;
    }

    void setPosted(bool posted)
    {
        QMutexLocker locker(&m_mutex);
        m_posted = posted;
    }

private:
    QMutex m_mutex;
    QList<std::shared_ptr<scopes::ResultItem>> m_results;
    bool m_finished;
    bool m_posted;
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

    ResultCollector* getCollector()
    {
        return m_collector.data();
    }

private:
    QSharedPointer<ResultCollector> m_collector;
};

const QEvent::Type PushEvent::eventType = static_cast<QEvent::Type>(QEvent::registerEventType());

class ResultReceiver: public scopes::ReceiverBase
{
public:
    virtual void push(scopes::ResultItem result) override
    {
        auto res = std::make_shared<scopes::ResultItem>(std::move(result));
        m_collector->addResult(res);
        // FIXME: we could post the event even here, but do we want that?
        postResults();
    }

    // which thread is this invoked in? (assuming same as push())
    virtual void finished() override
    {
        qWarning("query was finished");
        m_collector->setFinished(true);
        postResults();
    }

    void postResults()
    {
        if (m_collector->posted()) return;

        PushEvent* pushEvent = new PushEvent(m_collector);
        m_collector->setPosted(true);

        QMutexLocker locker(&m_receiver_lock);
        if (m_receiver) {
            // posting the event steals the ownership
            QCoreApplication::postEvent(m_receiver, pushEvent);
        } else {
            delete pushEvent;
        }
    }

    void invalidate()
    {
        QMutexLocker locker(&m_receiver_lock);
        m_receiver = nullptr;
    }

    ResultReceiver(QObject* receiver):
        m_collector(new ResultCollector),
        m_receiver(receiver)
    {
    }

private:
    QSharedPointer<ResultCollector> m_collector;
    QObject* m_receiver;
    QMutex m_receiver_lock;
};

Scope::Scope(QObject *parent) : QObject(parent)
    , m_formFactor("phone")
    , m_isActive(false)
    , m_searchInProgress(false)
{
    m_categories = new Categories(this);
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
        PushEvent* pe = static_cast<PushEvent*>(ev);
        auto collector = pe->getCollector();
        auto results = collector->getResults();
        qWarning("got a push event for %s: %d results", m_scopeId.toStdString().c_str(), results.count());
        if (collector->finished() && m_searchInProgress) {
            m_searchInProgress = false;
            Q_EMIT searchInProgressChanged();
        }
    }
    QObject::event(ev);
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
    return QString::fromStdString("Foo");
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
        if (m_lastReceiver) {
            std::dynamic_pointer_cast<ResultReceiver>(m_lastReceiver)->invalidate();
        }
        if (m_lastQuery) {
            m_lastQuery->cancel();
        }
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
