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
#include "collectors.h"
#include "preview.h"

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
#include <QScopedPointer>
#include <QFileInfo>
#include <QDir>

#include <libintl.h>
#include <glib.h>

#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/QueryCtrl.h>
#include <unity/scopes/PreviewWidget.h>

namespace scopes_ng
{

using namespace unity;

const int AGGREGATION_TIMEOUT = 110;

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
    if (m_lastSearch) {
        std::dynamic_pointer_cast<SearchResultReceiver>(m_lastSearch)->invalidate();
    }
    if (m_lastPreview) {
        std::dynamic_pointer_cast<PreviewDataReceiver>(m_lastPreview)->invalidate();
    }
}

void Scope::processSearchChunk(PushEvent* pushEvent)
{
    CollectorBase::Status status;
    QList<std::shared_ptr<scopes::CategorisedResult>> results;

    status = pushEvent->collectResults(results);
    if (status == CollectorBase::Status::CANCELLED) {
        return;
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

    if (status == CollectorBase::Status::INCOMPLETE) {
        if (!m_aggregatorTimer.isActive()) {
            m_aggregatorTimer.start(AGGREGATION_TIMEOUT);
        }
    } else { // status in [FINISHED, ERROR]
        m_aggregatorTimer.stop();

        flushUpdates();

        if (m_searchInProgress) {
            m_searchInProgress = false;
            Q_EMIT searchInProgressChanged();
        }
    }
}

void Scope::processPreviewChunk(PushEvent* pushEvent)
{
    CollectorBase::Status status;
    scopes::PreviewWidgetList widgets;
    QHash<QString, QVariant> preview_data;

    status = pushEvent->collectPreviewData(widgets, preview_data);
    if (status == CollectorBase::Status::CANCELLED) {
        return;
    }

    if (m_preview) {
        m_preview->addWidgetDefinitions(widgets);
        m_preview->updatePreviewData(preview_data);
    }

    // status in [FINISHED, ERROR]
    if (status != CollectorBase::Status::INCOMPLETE) {
        if (m_preview) {
            // FIXME: how to signal when preview finishes with error?
            Q_EMIT previewReady(m_preview);
            m_preview.clear();
        }
    }
}

bool Scope::event(QEvent* ev)
{
    if (ev->type() == PushEvent::eventType) {
        PushEvent* pushEvent = static_cast<PushEvent*>(ev);

        switch (pushEvent->type()) {
            case PushEvent::SEARCH:
                processSearchChunk(pushEvent);
                return true;
            case PushEvent::PREVIEW:
                processPreviewChunk(pushEvent);
                return true;
            default:
                qWarning("Unknown PushEvent type!");
                return false;
        }
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
    if (m_lastSearch) {
        std::dynamic_pointer_cast<SearchResultReceiver>(m_lastSearch)->invalidate();
        m_lastSearch.reset();
    }
    if (m_lastSearchQuery) {
        m_lastSearchQuery->cancel();
        m_lastSearchQuery.reset();
    }
    if (m_aggregatorTimer.isActive()) {
        m_aggregatorTimer.stop();
    }
    m_cachedResults.clear();

    // TODO: not the best idea to put the clearAll() here, can cause flicker
    m_categories->clearAll();
}

void Scope::invalidateLastPreview()
{
    if (m_lastPreview) {
        std::dynamic_pointer_cast<PreviewDataReceiver>(m_lastPreview)->invalidate();
        m_lastPreview.reset();
    }
    if (m_lastPreviewQuery) {
        m_lastPreviewQuery->cancel();
        m_lastPreviewQuery.reset();
    }
    if (m_preview) {
        m_preview.clear();
    }
}

void Scope::dispatchSearch()
{
    /* There are a few objects associated with searches:
     * 1) SearchResultReceiver    2) ResultCollector    3) PushEvent
     *
     * SearchResultReceiver is associated with the search and has methods that get called
     * by the scopes framework when results / categories / annotations are received.
     * Since the notification methods (push(...)) of SearchResultReceiver are called
     * from different thread(s), it uses ResultCollector to collect multiple results
     * in a thread-safe manner.
     * Once a couple of results are collected, the collector is sent via a PushEvent
     * to the UI thread, where it is processed. When the results are collected by the UI thread,
     * the collector continues to collect more results, and uses another PushEvent to post them.
     *
     * If a new query is submitted the previous one is marked as cancelled by invoking
     * SearchResultReceiver::invalidate() and any PushEvent that is waiting to be processed
     * will be discarded as the collector will also be marked as invalid.
     * The new query will have new instances of SearchResultReceiver and ResultCollector.
     */

    if (m_proxy) {
        scopes::VariantMap vm;
        m_lastSearch.reset(new SearchResultReceiver(this));
        try {
            m_lastSearchQuery = m_proxy->create_query(m_searchQuery.toStdString(), vm, m_lastSearch);
        } catch (...) {
            qWarning("Caught exception from create_query()");
        }
    }
}

PreviewModel* Scope::dispatchPreview(std::shared_ptr<scopes::Result> const& result)
{
    PreviewModel* preview = nullptr;
    if (m_proxy) {
        scopes::VariantMap vm;
        m_lastPreview.reset(new PreviewDataReceiver(this));
        preview = new PreviewModel(this);
        preview->setResult(result);
        // FIXME: don't block
        try {
            m_lastPreviewQuery = m_proxy->preview(*(result.get()), vm, m_lastPreview);
        } catch (...) {
            qWarning("Caught exception from preview()");
        }
    }

    return preview;
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

void Scope::activate(QVariant const& result_var)
{
    if (!result_var.canConvert<std::shared_ptr<scopes::Result>>()) {
        qWarning("Cannot activate result, unable to convert");
        return;
    }

    std::shared_ptr<scopes::Result> result = result_var.value<std::shared_ptr<scopes::Result>>();
    if (!result) {
        qWarning("Cannot activate null result");
        return;
    }

    if (result->direct_activation()) {
        activateUri(QString::fromStdString(result->uri()));
    } else if (m_scopeMetadata) {
        try {
            auto scope_name = result->activation_scope_name();
            if (scope_name == m_scopeMetadata->scope_name()) {
                // FIXME: don't block, handle the result
                m_proxy->activate(*(result.get()), scopes::VariantMap(), nullptr);
            } else {
                // FIXME: send the request to a different proxy
                qWarning("UNIMPLEMENTED: result needs to be activated by '%s'", scope_name.c_str());
            }
        } catch (...) {
            qWarning("Caught an error while activating result");
        }
    } else {
        qWarning("Unable to activate result");
    }
}

PreviewModel* Scope::preview(QVariant const& result_var)
{
    if (!result_var.canConvert<std::shared_ptr<scopes::Result>>()) {
        qWarning("Cannot preview result, unable to convert");
        return nullptr;
    }

    std::shared_ptr<scopes::Result> result = result_var.value<std::shared_ptr<scopes::Result>>();
    if (!result) {
        qWarning("Cannot preview null result");
        return nullptr;
    }

    // TODO: figure out if the result can produce a preview without sending a request to the scope
    // if (result->has_early_preview()) { ... }
    if (m_scopeMetadata) {
        auto scope_name = result->activation_scope_name();
        invalidateLastPreview();
        if (scope_name == m_scopeMetadata->scope_name()) {
            m_preview = dispatchPreview(result);
        } else {
            // FIXME: send the request to a different proxy
            qWarning("UNIMPLEMENTED: result needs to be activated by '%s'", scope_name.c_str());
        }
    } else {
        qWarning("Unable to activate result");
    }
    return m_preview;
}

void Scope::cancelActivation()
{
}

void Scope::activateUri(QString const& uri)
{
    /* Tries various methods to trigger a sensible action for the given 'uri'.
       If it has no understanding of the given scheme it falls back on asking
       Qt to open the uri.
    */
    QUrl url(uri);
    if (url.scheme() == "application") {
        QString path(url.path().isEmpty() ? url.authority() : url.path());
        if (path.startsWith("/")) {
            Q_FOREACH(const QString &dir, QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation)) {
                if (path.startsWith(dir)) {
                    path.remove(dir);
                    path.replace('/', '-');
                    break;
                }
            }
        }

        Q_EMIT activateApplication(QFileInfo(path).completeBaseName());
    } else {
        qDebug() << "Trying to open" << uri;
        /* Try our luck */
        QDesktopServices::openUrl(url);
    }
}

} // namespace scopes_ng
