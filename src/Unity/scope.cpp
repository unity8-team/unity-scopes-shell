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
#include "previewstack.h"
#include "utils.h"
#include "scopes.h"

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
#include <QLocale>

#include <libintl.h>

#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/QueryCtrl.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/scopes/ActionMetadata.h>

namespace scopes_ng
{

using namespace unity;

const int AGGREGATION_TIMEOUT = 110;
const int CLEAR_TIMEOUT = 240;
const int RESULTS_TTL_SMALL = 30000; // 30 seconds
const int RESULTS_TTL_MEDIUM = 300000; // 5 minutes
const int RESULTS_TTL_LARGE = 3600000; // 1 hour

Scope::Scope(QObject *parent) : unity::shell::scopes::ScopeInterface(parent)
    , m_formFactor("phone")
    , m_isActive(false)
    , m_searchInProgress(false)
    , m_resultsDirty(false)
    , m_delayedClear(false)
    , m_hasDepartments(false)
    , m_searchController(new CollectionController)
    , m_activationController(new CollectionController)
{
    m_categories = new Categories(this);

    m_settings = QGSettings::isSchemaInstalled("com.canonical.Unity.Lenses") ? new QGSettings("com.canonical.Unity.Lenses", QByteArray(), this) : nullptr;
    QObject::connect(m_settings, &QGSettings::changed, this, &Scope::internetFlagChanged);

    m_aggregatorTimer.setSingleShot(true);
    QObject::connect(&m_aggregatorTimer, &QTimer::timeout, this, &Scope::flushUpdates);
    m_clearTimer.setSingleShot(true);
    QObject::connect(&m_clearTimer, &QTimer::timeout, this, &Scope::flushUpdates);
    m_invalidateTimer.setSingleShot(true);
    m_invalidateTimer.setTimerType(Qt::VeryCoarseTimer);
    QObject::connect(&m_invalidateTimer, &QTimer::timeout, this, &Scope::invalidateResults);
}

Scope::~Scope()
{
}

void Scope::processSearchChunk(PushEvent* pushEvent)
{
    CollectorBase::Status status;
    QList<std::shared_ptr<scopes::CategorisedResult>> results;
    scopes::Department::SPtr rootDepartment;
    scopes::Department::SPtr activeDepartment;

    status = pushEvent->collectSearchResults(results, rootDepartment, activeDepartment);
    if (status == CollectorBase::Status::CANCELLED) {
        return;
    }

    m_rootDepartment = rootDepartment;
    m_activeDepartment = activeDepartment;

    if (m_cachedResults.empty()) {
        m_cachedResults.swap(results);
    } else {
        m_cachedResults.append(results);
    }

    if (status == CollectorBase::Status::INCOMPLETE) {
        if (!m_aggregatorTimer.isActive()) {
            // the longer we've been waiting for the results, the shorter the timeout
            qint64 inProgressMs = pushEvent->msecsSinceStart();
            double mult = 1.0 / std::max(1, static_cast<int>((inProgressMs / 150) + 1));
            m_aggregatorTimer.start(AGGREGATION_TIMEOUT * mult);
        }
    } else { // status in [FINISHED, ERROR]
        m_aggregatorTimer.stop();

        flushUpdates();

        setSearchInProgress(false);

        // Don't schedule a refresh if the query suffered an error
        if (status == CollectorBase::Status::FINISHED) {
            startTtlTimer();
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
            case PushEvent::ACTIVATION: {
                std::shared_ptr<scopes::ActivationResponse> response;
                std::shared_ptr<scopes::Result> result;
                pushEvent->collectActivationResponse(response, result);
                if (response) {
                    handleActivation(response, result);
                }
                return true;
            }
            default:
                qWarning("Unknown PushEvent type!");
                return false;
        }
    }
    return QObject::event(ev);
}

void Scope::handleActivation(std::shared_ptr<scopes::ActivationResponse> const& response, scopes::Result::SPtr const& result)
{
    switch (response->status()) {
        case scopes::ActivationResponse::NotHandled:
            activateUri(QString::fromStdString(result->uri()));
            break;
        case scopes::ActivationResponse::HideDash:
            Q_EMIT hideDash();
            break;
        case scopes::ActivationResponse::ShowDash:
            Q_EMIT showDash();
            break;
        case scopes::ActivationResponse::ShowPreview:
            Q_EMIT previewRequested(QVariant::fromValue(result));
            break;
         case scopes::ActivationResponse::PerformQuery:
            executeCannedQuery(response->query(), true);
            break;
        default:
            break;
    }
}

void Scope::metadataRefreshed()
{
    std::shared_ptr<scopes::ActivationResponse> response;
    response.swap(m_delayedActivation);

    if (!response) {
        return;
    }

    if (response->status() == scopes::ActivationResponse::PerformQuery) {
        executeCannedQuery(response->query(), false);
    }
}

void Scope::internetFlagChanged(QString const& key)
{
    if (key != "remoteContentSearch") {
        return;
    }

    invalidateResults();
}

void Scope::executeCannedQuery(unity::scopes::CannedQuery const& query, bool allowDelayedActivation)
{
    scopes_ng::Scopes* scopes = qobject_cast<scopes_ng::Scopes*>(parent());
    if (scopes == nullptr) {
        qWarning("Scope instance %p doesn't have Scopes as a parent", static_cast<void*>(this));
        return;
    }

    QString scopeId(QString::fromStdString(query.scope_id()));
    QString searchString(QString::fromStdString(query.query_string()));
    // figure out if this scope is already favourited
    Scope* scope = scopes->getScopeById(scopeId);
    if (scope != nullptr) {
        // TODO: change department, filters?
        scope->setSearchQuery(searchString);
        Q_EMIT gotoScope(scopeId);
    } else {
        // create temp dash page
        auto meta_sptr = scopes->getCachedMetadata(scopeId);
        if (meta_sptr) {
            scope = new scopes_ng::Scope(this);
            scope->setScopeData(*meta_sptr);
            scope->setSearchQuery(searchString);
            m_tempScopes.insert(scope);
            Q_EMIT openScope(scope);
        } else if (allowDelayedActivation) {
            // request registry refresh to get the missing metadata
            m_delayedActivation = std::make_shared<scopes::ActivationResponse>(query);
            QObject::connect(scopes, &Scopes::metadataRefreshed, this, &Scope::metadataRefreshed);
            scopes->refreshScopeMetadata();
        } else {
            qWarning("Unable to find scope \"%s\" after metadata refresh", query.scope_id().c_str());
        }
    }
}

void Scope::flushUpdates()
{
    if (m_delayedClear) {
        // TODO: here we could do resultset diffs
        m_categories->clearAll();
        m_delayedClear = false;
    }

    if (m_clearTimer.isActive()) {
        m_clearTimer.stop();
    }

    processResultSet(m_cachedResults); // clears the result list

    // process departments
    if (m_rootDepartment != m_lastRootDepartment || m_activeDepartment != m_lastActiveDepartment) {
        // build / append to the tree
        DepartmentNode* node = nullptr;
        if (m_departmentTree) {
            QString departmentId(QString::fromStdString(m_rootDepartment->id()));
            node = m_departmentTree->findNodeById(departmentId);
            // FIXME: uuuuh, could be null!
            node->initializeForDepartment(m_rootDepartment);
            // as far as we know, this is the root, re-initializing might have unset the flag
            m_departmentTree->setIsRoot(true);

            // update corresponding models
            QString activeDepartment(QString::fromStdString(m_activeDepartment->id()));
            node = m_departmentTree->findNodeById(activeDepartment);
            DepartmentNode* parentNode = nullptr;
            if (node != nullptr) {
                auto it = m_departmentModels.find(activeDepartment);
                while (it != m_departmentModels.end() && it.key() == activeDepartment) {
                    it.value()->loadFromDepartmentNode(node);
                    ++it;
                }
                // if this node is a leaf, we need to update models for the parent
                parentNode = node->isLeaf() ? node->parent() : nullptr;
            }
            if (parentNode != nullptr) {
                auto it = m_departmentModels.find(parentNode->id());
                while (it != m_departmentModels.end() && it.key() == parentNode->id()) {
                    it.value()->markSubdepartmentActive(activeDepartment);
                    ++it;
                }
            }
        } else {
            m_departmentTree.reset(new DepartmentNode);
            m_departmentTree->initializeForDepartment(m_rootDepartment);
            // as far as we know, this is the root, changing our mind later
            // is better than pretending it isn't
            m_departmentTree->setIsRoot(true);
        }

        m_lastRootDepartment = m_rootDepartment;
        m_lastActiveDepartment = m_activeDepartment;
    }

    QString activeDepId;
    if (m_activeDepartment) {
        activeDepId = QString::fromStdString(m_activeDepartment->id());
    }

    bool containsDepartments = m_rootDepartment.get() != nullptr;
    if (containsDepartments != m_hasDepartments) {
        m_hasDepartments = containsDepartments;
        Q_EMIT hasDepartmentsChanged();
    }

    if (activeDepId != m_currentDepartmentId) {
        m_currentDepartmentId = activeDepId;
        Q_EMIT currentDepartmentIdChanged();
    }
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
    m_searchController->invalidate();
    if (m_aggregatorTimer.isActive()) {
        m_aggregatorTimer.stop();
    }
    m_cachedResults.clear();
}

void Scope::startTtlTimer()
{
    if (m_scopeMetadata) {
        int ttl = 0;
        switch (m_scopeMetadata->results_ttl_type()) {
        case (scopes::ScopeMetadata::ResultsTtlType::None):
            break;
        case (scopes::ScopeMetadata::ResultsTtlType::Small):
            ttl = RESULTS_TTL_SMALL;
            break;
        case (scopes::ScopeMetadata::ResultsTtlType::Medium):
            ttl = RESULTS_TTL_MEDIUM;
            break;
        case (scopes::ScopeMetadata::ResultsTtlType::Large):
            ttl = RESULTS_TTL_LARGE;
            break;
        }
        if (ttl > 0) {
            if (qEnvironmentVariableIsSet("UNITY_SCOPES_RESULTS_TTL_OVERRIDE")) {
                ttl = QString::fromUtf8(
                        qgetenv("UNITY_SCOPES_RESULTS_TTL_OVERRIDE")).toInt();
            }
            m_invalidateTimer.start(ttl);
        }
    }
}

void Scope::setSearchInProgress(bool searchInProgress)
{
    if (m_searchInProgress != searchInProgress) {
        m_searchInProgress = searchInProgress;
        Q_EMIT searchInProgressChanged();
    }
}

void Scope::dispatchSearch()
{
    invalidateLastSearch();
    m_delayedClear = true;
    m_clearTimer.start(CLEAR_TIMEOUT);
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

    if (m_resultsDirty)
    {
        m_resultsDirty = false;
        resultsDirtyChanged();
    }

    setSearchInProgress(true);

    if (m_proxy) {
        scopes::SearchMetadata meta(QLocale::system().name().toStdString(), m_formFactor.toStdString());
        if (m_settings) {
            QVariant remoteSearch(m_settings->get("remote-content-search"));
            if (remoteSearch.toString() == QString("none")) {
                meta["no-internet"] = true;
            }
        }
        scopes::SearchListenerBase::SPtr listener(new SearchResultReceiver(this));
        m_searchController->setListener(listener);
        try {
            scopes::QueryCtrlProxy controller = m_proxy->search(m_searchQuery.toStdString(), m_currentDepartmentId.toStdString(), scopes::FilterState(), meta, listener);
            m_searchController->setController(controller);
        } catch (std::exception& e) {
            qWarning("Caught an error from create_query(): %s", e.what());
        } catch (...) {
            qWarning("Caught an error from create_query()");
        }
    }

    if (!m_searchController->isValid()) {
        // something went wrong, reset search state
        setSearchInProgress(false);
    }
}

void Scope::setScopeData(scopes::ScopeMetadata const& data)
{
    m_scopeMetadata = std::make_shared<scopes::ScopeMetadata>(data);
    m_proxy = data.proxy();
}

QString Scope::id() const
{
    return QString::fromStdString(m_scopeMetadata ? m_scopeMetadata->scope_id() : "");
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

unity::shell::scopes::CategoriesInterface* Scope::categories() const
{
    return m_categories;
}

/*
Filters* Scope::filters() const
{
    return m_filters.get();
}
*/

unity::shell::scopes::DepartmentInterface* Scope::getDepartment(QString const& departmentId)
{
    if (!m_departmentTree) return nullptr;

    DepartmentNode* node = m_departmentTree->findNodeById(departmentId);
    if (!node) return nullptr;

    Department* departmentModel = new Department;
    departmentModel->loadFromDepartmentNode(node);

    m_departmentModels.insert(departmentId, departmentModel);
    m_inverseDepartments.insert(departmentModel, departmentId);
    QObject::connect(departmentModel, &QObject::destroyed, this, &Scope::departmentModelDestroyed);

    return departmentModel;
}

void Scope::departmentModelDestroyed(QObject* obj)
{
  scopes_ng::Department* department = reinterpret_cast<scopes_ng::Department*>(obj);

  auto it = m_inverseDepartments.find(department);
  if (it == m_inverseDepartments.end()) return;

  m_departmentModels.remove(it.value(), department);
  m_inverseDepartments.remove(department);
}

void Scope::loadDepartment(QString const& departmentId)
{
    if (departmentId != m_currentDepartmentId) {
        m_currentDepartmentId = departmentId;
        Q_EMIT currentDepartmentIdChanged();

        dispatchSearch();
    }
}

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

QString Scope::currentDepartmentId() const
{
    return m_currentDepartmentId;
}

bool Scope::hasDepartments() const
{
    return m_hasDepartments;
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

        // FIXME: use a timeout
        dispatchSearch();

        Q_EMIT searchQueryChanged();
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
        Q_EMIT isActiveChanged();

        if (active && m_resultsDirty) {
            dispatchSearch();
        }
    }
}

void Scope::activate(QVariant const& result_var)
{
    if (!result_var.canConvert<std::shared_ptr<scopes::Result>>()) {
        qWarning("Cannot activate, unable to convert %s to Result", result_var.typeName());
        return;
    }

    std::shared_ptr<scopes::Result> result = result_var.value<std::shared_ptr<scopes::Result>>();
    if (!result) {
        qWarning("activate(): received null result");
        return;
    }

    if (result->direct_activation()) {
        activateUri(QString::fromStdString(result->uri()));
    } else {
        try {
            cancelActivation();
            scopes::ActivationListenerBase::SPtr listener(new ActivationReceiver(this, result));
            m_activationController->setListener(listener);

            auto proxy = result->target_scope_proxy();
            unity::scopes::ActionMetadata metadata(QLocale::system().name().toStdString(), m_formFactor.toStdString());
            scopes::QueryCtrlProxy controller = proxy->activate(*(result.get()), metadata, listener);
            m_activationController->setController(controller);
        } catch (std::exception& e) {
            qWarning("Caught an error from activate(): %s", e.what());
        } catch (...) {
            qWarning("Caught an error from activate()");
        }
    }
}

unity::shell::scopes::PreviewStackInterface* Scope::preview(QVariant const& result_var)
{
    if (!result_var.canConvert<std::shared_ptr<scopes::Result>>()) {
        qWarning("Cannot preview, unable to convert %s to Result", result_var.typeName());
        return nullptr;
    }

    scopes::Result::SPtr result = result_var.value<std::shared_ptr<scopes::Result>>();
    if (!result) {
        qWarning("preview(): received null result");
        return nullptr;
    }

    PreviewStack* stack = new PreviewStack(nullptr);
    stack->setAssociatedScope(this);
    stack->loadForResult(result);
    return stack;
}

void Scope::cancelActivation()
{
    m_activationController->invalidate();
}

void Scope::invalidateResults()
{
    if (m_isActive) {
        dispatchSearch();
    } else {
        // mark the results as dirty, so next setActive() re-sends the query
        if (!m_resultsDirty)
        {
            m_resultsDirty = true;
            resultsDirtyChanged();
        }
    }
}

void Scope::closeScope(unity::shell::scopes::ScopeInterface* scope)
{
    if (m_tempScopes.remove(scope)) {
        delete scope;
    }
}

bool Scope::resultsDirty() const {
    return m_resultsDirty;
}

void Scope::activateUri(QString const& uri)
{
    /* Tries various methods to trigger a sensible action for the given 'uri'.
       If it has no understanding of the given scheme it falls back on asking
       Qt to open the uri.
    */
    QUrl url(uri);
    if (url.scheme() == QLatin1String("application")) {
        QString path(url.path().isEmpty() ? url.authority() : url.path());
        if (path.startsWith("/")) {
            Q_FOREACH(const QString &dir, QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation)) {
                if (path.startsWith(dir)) {
                    path.remove(0, dir.length());
                    path.replace('/', '-');
                    break;
                }
            }
        }

        Q_EMIT activateApplication(QFileInfo(path).completeBaseName());
    } else if (url.scheme() == QLatin1String("scope")) {
        qDebug() << "Got scope URI" << uri;
        try {
            scopes::CannedQuery q(scopes::CannedQuery::from_uri(uri.toStdString()));
            executeCannedQuery(q, true);
        } catch (...) {
            qWarning("Unable to parse scope uri!");
        }
    } else {
        qDebug() << "Trying to open" << uri;
        /* Try our luck */
        QDesktopServices::openUrl(url);
    }
}

} // namespace scopes_ng
