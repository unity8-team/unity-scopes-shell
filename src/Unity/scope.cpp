/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * Authors:
 *  Michal Hruby <michal.hruby@canonical.com>
 *  Pawel Stolowski <pawel.stolowski@canonical.com>
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
#include "previewmodel.h"
#include "locationservice.h"
#include "utils.h"
#include "scopes.h"
#include "settingsmodel.h"
#include "logintoaccount.h"

// Qt
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QtGui/QDesktopServices>
#include <QEvent>
#include <QMutex>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QElapsedTimer>
#include <QScopedPointer>
#include <QFileInfo>
#include <QDir>
#include <QLocale>
#include <QtConcurrent>

#include <QQmlEngine>

#include <libintl.h>

#include <online-accounts-client/Setup>

#include <unity/scopes/ListenerBase.h>
#include <unity/scopes/CannedQuery.h>
#include <unity/scopes/OptionSelectorFilter.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/QueryCtrl.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/SearchMetadata.h>
#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/Variant.h>

namespace scopes_ng
{

using namespace unity;

const int TYPING_TIMEOUT = 700;
const int SEARCH_PROCESSING_DELAY = 1000;
const int RESULTS_TTL_SMALL = 30000; // 30 seconds
const int RESULTS_TTL_MEDIUM = 300000; // 5 minutes
const int RESULTS_TTL_LARGE = 3600000; // 1 hour
const int SEARCH_CARDINALITY = 300; // maximum number of results accepted from a single scope

Scope::Ptr Scope::newInstance(scopes_ng::Scopes* parent)
{
    return Scope::Ptr(new Scope(parent), &QObject::deleteLater);
}

Scope::Scope(scopes_ng::Scopes* parent) :
      m_query_id(0)
    , m_formFactor(QStringLiteral("phone"))
    , m_activeFiltersCount(0)
    , m_isActive(false)
    , m_searchInProgress(false)
    , m_activationInProgress(false)
    , m_resultsDirty(false)
    , m_delayedSearchProcessing(false)
    , m_hasNavigation(false)
    , m_favorite(false)
    , m_initialQueryDone(false)
    , m_childScopesDirty(true)
    , m_searchController(new CollectionController)
    , m_activationController(new CollectionController)
    , m_status(Status::Okay)
{
    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);
    m_categories.reset(new Categories(this));
    m_filters.reset(new Filters(m_filterState, this));

    connect(m_filters.data(), SIGNAL(primaryFilterChanged()), this, SIGNAL(primaryNavigationFilterChanged()));

    QQmlEngine::setObjectOwnership(m_filters.data(), QQmlEngine::CppOwnership);
    connect(m_filters.data(), SIGNAL(filterStateChanged()), this, SLOT(filterStateChanged()));

    setScopesInstance(parent);

    m_typingTimer.setSingleShot(true);
    if (qEnvironmentVariableIsSet("UNITY_SCOPES_TYPING_TIMEOUT_OVERRIDE"))
    {
        m_typingTimer.setInterval(QString::fromUtf8(qgetenv("UNITY_SCOPES_TYPING_TIMEOUT_OVERRIDE")).toInt());
    }
    else
    {
        m_typingTimer.setInterval(TYPING_TIMEOUT);
    }
    if (qEnvironmentVariableIsSet("UNITY_SCOPES_CARDINALITY_OVERRIDE")) {
        m_cardinality = qgetenv("UNITY_SCOPES_CARDINALITY_OVERRIDE").toInt();
    } else {
        m_cardinality = SEARCH_CARDINALITY;
    }
    QObject::connect(&m_typingTimer, &QTimer::timeout, this, &Scope::typingFinished);
    m_searchProcessingDelayTimer.setSingleShot(true);
    QObject::connect(&m_searchProcessingDelayTimer, SIGNAL(timeout()), this, SLOT(flushUpdates()));
    m_invalidateTimer.setSingleShot(true);
    m_invalidateTimer.setTimerType(Qt::CoarseTimer);
    QObject::connect(&m_invalidateTimer, &QTimer::timeout, this, &Scope::invalidateResults);
}

Scope::~Scope()
{
    m_childScopesFuture.waitForFinished();
}

void Scope::processSearchChunk(PushEvent* pushEvent)
{
    CollectorBase::Status status;
    QList<std::shared_ptr<scopes::CategorisedResult>> results;
    scopes::Department::SCPtr rootDepartment;
    QList<scopes::FilterBase::SCPtr> filters;

    status = pushEvent->collectSearchResults(results, rootDepartment, filters);
    if (status == CollectorBase::Status::CANCELLED) {
        return;
    }

    m_rootDepartment = rootDepartment;
    m_receivedFilters = filters;

    if (m_cachedResults.empty()) {
        m_cachedResults.swap(results);
    } else {
        m_cachedResults.append(results);
    }

    if (status == CollectorBase::Status::INCOMPLETE) {
        if (!m_searchProcessingDelayTimer.isActive()) {
            // the longer we've been waiting for the results, the shorter the timeout
            qint64 inProgressMs = pushEvent->msecsSinceStart();
            double mult = 1.0 / std::max(1, static_cast<int>((inProgressMs / 150) + 1));
            m_searchProcessingDelayTimer.start(SEARCH_PROCESSING_DELAY * mult);
        }
    } else { // status in [FINISHED, ERROR]
        m_searchProcessingDelayTimer.stop();

        flushUpdates(true);

        setSearchInProgress(false);

        switch (status) {
            case CollectorBase::Status::FINISHED:
            case CollectorBase::Status::CANCELLED:
                setStatus(Status::Okay);
                break;
            case CollectorBase::Status::NO_INTERNET:
                setStatus(Status::NoInternet);
                break;
            case CollectorBase::Status::NO_LOCATION_DATA:
                setStatus(Status::NoLocationData);
                break;
            default:
                setStatus(Status::Unknown);
        }

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
                QString categoryId;
                pushEvent->collectActivationResponse(response, result, categoryId);
                if (response) {
                    handleActivation(response, result, categoryId);
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

void Scope::handleActivation(std::shared_ptr<scopes::ActivationResponse> const& response, scopes::Result::SPtr const& result, QString const& categoryId)
{
    setActivationInProgress(false);

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
        case scopes::ActivationResponse::UpdateResult:
            m_categories->updateResult(*result, categoryId, response->updated_result());
            Q_EMIT updateResultRequested();
            break;
        case scopes::ActivationResponse::UpdatePreview:
            handlePreviewUpdate(result, response->updated_widgets());
            break;
        default:
            break;
    }
}

void Scope::metadataRefreshed()
{
    // refresh Settings view if needed
    if (require_child_scopes_refresh()) {
        m_childScopesDirty = true;
        update_child_scopes();
    }

    std::shared_ptr<scopes::ActivationResponse> response;
    response.swap(m_delayedActivation);

    if (!response) {
        return;
    }

    if (response->status() == scopes::ActivationResponse::PerformQuery) {
        executeCannedQuery(response->query(), false);
    }
}

void Scope::setCannedQuery(unity::scopes::CannedQuery const& query)
{
    setCurrentNavigationId(QString::fromStdString(query.department_id()));
    setFilterState(query.filter_state());
    if (query.has_user_data()) {
        m_queryUserData.reset(new unity::scopes::Variant(query.user_data()));
    }
    else
    {
        m_queryUserData.reset(nullptr);
    }
    setSearchQueryString(QString::fromStdString(query.query_string()));
}

void Scope::handlePreviewUpdate(unity::scopes::Result::SPtr const& result, unity::scopes::PreviewWidgetList const& widgets)
{
    for (auto model: m_previewModels) {
        auto previewedResult = model->previewedResult();

        if (result == nullptr) {
            qWarning() << "handlePreviewUpdate: result is null";
            return;
        }
        if (previewedResult != nullptr && *result == *previewedResult) {
            model->update(widgets);
        }
    }
}

void Scope::executeCannedQuery(unity::scopes::CannedQuery const& query, bool allowDelayedActivation)
{
    if (!m_scopesInstance) {
        qWarning("Scope instance %p doesn't have associated Scopes instance", static_cast<void*>(this));
        return;
    }

    const QString scopeId(QString::fromStdString(query.scope_id()));

    Scope* scope = nullptr;
    if (scopeId == id()) {
        scope = this;
    } else {
        // figure out if this scope is already favourited
        auto tmp = m_scopesInstance->getScopeById(scopeId);
        if (tmp) {
           scope = tmp.data();
        }
    }

    if (scope) {
        scope->setCannedQuery(query);
        scope->invalidateResults();

        if (scope != this) {
            Q_EMIT gotoScope(scopeId);
        } else {
            Q_EMIT showDash();
        }
    } else {
        // create temp dash page
        auto meta_sptr = m_scopesInstance->getCachedMetadata(scopeId);
        if (meta_sptr) {
            Scope::Ptr newScope = Scope::newInstance(m_scopesInstance);
            newScope->setScopeData(*meta_sptr);
            newScope->setCannedQuery(query);
            m_scopesInstance->addTempScope(newScope);
            Q_EMIT openScope(newScope.data());
        } else if (allowDelayedActivation) {
            // request registry refresh to get the missing metadata
            m_delayedActivation = std::make_shared<scopes::ActivationResponse>(query);
            m_scopesInstance->refreshScopeMetadata();
        } else {
            qWarning("Unable to find scope \"%s\" after metadata refresh", qPrintable(scopeId));
            Q_EMIT activationFailed(scopeId);
        }
    }
}

void Scope::typingFinished()
{
    invalidateResults();

    Q_EMIT searchQueryChanged();
}

void Scope::flushUpdates(bool finalize)
{
    if (m_delayedSearchProcessing) {
        m_delayedSearchProcessing = false;
    }

    if (m_status != Status::Okay) {
        setStatus(Status::Okay);
    }

    // if no results have been received so far (and we're not in the finalizing step of search), then
    // don't process the results as this will inevitably make the dash empty.
    if (m_cachedResults.empty() && !finalize) {
        return;
    }

#ifdef VERBOSE_MODEL_UPDATES
    qDebug() << "flushUpdates:" << id() << "#results =" << m_cachedResults.count() << "finalize:" << finalize;
#endif

    processResultSet(m_cachedResults); // clears the result list

    if (finalize) {
        m_category_results.clear();
        m_categories->purgeResults(); // remove results for categories which were not present in new resultset
    }

    // process departments
    if (m_rootDepartment && m_rootDepartment != m_lastRootDepartment) {
        // build / append to the tree
        DepartmentNode* node = nullptr;
        if (m_departmentTree) {
            scopes::Department::SCPtr updateNode(m_rootDepartment);
            QString departmentId(QString::fromStdString(updateNode->id()));
            node = m_departmentTree->findNodeById(departmentId);
            if (node == nullptr) {
                node = m_departmentTree.data();
            } else {
                // we have the node in our tree, try to find the minimal subtree to update
                updateNode = findUpdateNode(node, updateNode);
                if (updateNode) {
                    node = m_departmentTree->findNodeById(QString::fromStdString(updateNode->id()));
                }
            }
            if (updateNode) {
                node->initializeForDepartment(updateNode);
            }
            // as far as we know, this is the root, re-initializing might have unset the flag
            m_departmentTree->setIsRoot(true);

            // update corresponding models
            updateNavigationModels(m_departmentTree.data(), m_departmentModels, m_currentNavigationId);
        } else {
            m_departmentTree.reset(new DepartmentNode);
            m_departmentTree->initializeForDepartment(m_rootDepartment);
            // as far as we know, this is the root, changing our mind later
            // is better than pretending it isn't
            m_departmentTree->setIsRoot(true);
        }
    }

    m_lastRootDepartment = m_rootDepartment;
    bool containsDepartments = (m_rootDepartment.get() != nullptr);

    //
    // only consider resetting current department id if we are in final flushUpdates
    // or received departments already. We don't know if we should reset it
    // until query finishes because departments may still arrive.
    if (finalize || containsDepartments)
    {
        if (containsDepartments != m_hasNavigation) {
            m_hasNavigation = containsDepartments;
            Q_EMIT hasNavigationChanged();
        }

        if (!containsDepartments && !m_currentNavigationId.isEmpty()) {
            qDebug() << "Resetting current nav id";
            m_currentNavigationId = QLatin1String("");
            Q_EMIT currentNavigationIdChanged();
        }
        processPrimaryNavigationTag(m_currentNavigationId);
    }

    // process filters
    if (finalize || m_receivedFilters.size() > 0)
    {
        qDebug() << "Processing" << m_receivedFilters.size() << "filters";
        const bool containsFilters = (m_receivedFilters.size() > 0);
        const bool haveFiltersAlready = (m_filters->rowCount() > 0);
        if (containsFilters) {
            m_filters->update(m_receivedFilters, containsDepartments);
            processPrimaryNavigationTag(m_currentNavigationId);
            if (!haveFiltersAlready) {
                Q_EMIT filtersChanged();
            }
            qDebug() << "Current number of filters:" << m_filters->rowCount();
        }
        else
        {
            qDebug() << "Removing all filters";
            m_filters->clear();
            if (haveFiltersAlready) {
                Q_EMIT filtersChanged();
            }
        }
    }
}

Scope::Ptr Scope::findTempScope(QString const& id) const
{
    if (m_scopesInstance) {
        return m_scopesInstance->findTempScope(id);
    }
    return Scope::Ptr();
}

void Scope::updateNavigationModels(DepartmentNode* rootNode, QMultiMap<QString, Department*>& navigationModels, QString const& activeNavigation)
{
    DepartmentNode* parentNode = nullptr;
    DepartmentNode* node = rootNode->findNodeById(activeNavigation);
    if (node != nullptr) {
        auto it = navigationModels.find(activeNavigation);
        while (it != navigationModels.end() && it.key() == activeNavigation) {
            it.value()->loadFromDepartmentNode(node);
            ++it;
        }
        // if this node is a leaf, we need to update models for the parent
        parentNode = node->isLeaf() ? node->parent() : nullptr;
    }
    if (parentNode != nullptr) {
        auto it = navigationModels.find(parentNode->id());
        while (it != navigationModels.end() && it.key() == parentNode->id()) {
            it.value()->markSubdepartmentActive(activeNavigation);
            ++it;
        }
    }
}

scopes::Department::SCPtr Scope::findUpdateNode(DepartmentNode* node, scopes::Department::SCPtr const& scopeNode)
{
    if (node == nullptr || node->id() != QString::fromStdString(scopeNode->id())) return scopeNode;

    // are all the children in our cache?
    QStringList cachedChildrenIds;
    const auto childNodes = node->childNodes();
    cachedChildrenIds.reserve(childNodes.count());
    Q_FOREACH(DepartmentNode* child, childNodes) {
        cachedChildrenIds << child->id();
    }

    auto subdeps = scopeNode->subdepartments();
    QMap<QString, scopes::Department::SCPtr> childIdMap;
    for (auto it = subdeps.begin(); it != subdeps.end(); ++it) {
        QString childId = QString::fromStdString((*it)->id());
        childIdMap.insert(childId, *it);
        if (!cachedChildrenIds.contains(childId)) {
            return scopeNode;
        }
    }

    scopes::Department::SCPtr firstMismatchingChild;

    Q_FOREACH(DepartmentNode* child, node->childNodes()) {
        scopes::Department::SCPtr scopeChildNode(childIdMap[child->id()]);
        // the cache might have more data than the node, should we consider that bad?
        if (!scopeChildNode) {
            continue;
        }
        scopes::Department::SCPtr updateNode = findUpdateNode(child, scopeChildNode);
        if (updateNode) {
            if (!firstMismatchingChild) {
                firstMismatchingChild = updateNode;
            } else {
                // there are multiple mismatching children, update the entire node
                return scopeNode;
            }
        }
    }

    // department has been removed (not reported by scope); make sure it's only treated as such when
    // we're examining children of *current* department, othwerwise it would break on partial trees when visiting a leaf.
    if (firstMismatchingChild == nullptr && (int) scopeNode->subdepartments().size() < cachedChildrenIds.size() &&
            m_currentNavigationId.toStdString() == scopeNode->id()) {
        return scopeNode;
    }

    return firstMismatchingChild; // will be nullptr if everything matches
}

scopes::Department::SCPtr Scope::findDepartmentById(scopes::Department::SCPtr const& root, std::string const& id)
{
    if (root->id() == id) return root;

    auto sub_deps = root->subdepartments();
    for (auto it = sub_deps.begin(); it != sub_deps.end(); ++it) {
        if ((*it)->id() == id) {
            return *it;
        } else {
            auto node = findDepartmentById(*it, id);
            if (node) return node;
        }
    }

    return nullptr;
}

void Scope::processResultSet(QList<std::shared_ptr<scopes::CategorisedResult>>& result_set)
{
    if (result_set.count() == 0) return;

    // this will keep the list of categories in order
    QVector<scopes::Category::SCPtr> categories;

    // split the result_set by category_id; note that processResultSet may get called more than once
    // for single search request, all the contents of m_category_results accumulate until new search
    // is requested, so that addUpdateResults() can properly update affected models.
    while (!result_set.empty()) {
        auto result = result_set.takeFirst();
        if (!categories.contains(result->category())) {
            categories.append(result->category());
        }
        m_category_results[result->category()->id()].append(std::move(result));
    }

    Q_FOREACH(scopes::Category::SCPtr const& category, categories) {
        QSharedPointer<ResultsModel> category_model = m_categories->lookupCategory(category->id());
        if (category_model == nullptr) {
            category_model.reset(new ResultsModel(m_categories.data()));
            category_model->setCategoryId(QString::fromStdString(category->id()));
            category_model->addResults(m_category_results[category->id()]); // de-duplicates m_category_results
            m_categories->registerCategory(category, category_model);
        } else {
            m_categories->registerCategory(category, QSharedPointer<ResultsModel>());
            category_model->addUpdateResults(m_category_results[category->id()]); // de-duplicates m_category_results
            m_categories->updateResultCount(category_model);
        }
    }
}

scopes::ScopeProxy Scope::proxy() const
{
    return m_proxy;
}

scopes::ScopeProxy Scope::proxy_for_result(scopes::Result::SPtr const& result) const
{
    return result->target_scope_proxy();
}

void Scope::invalidateLastSearch()
{
    m_searchController->invalidate();
    if (m_searchProcessingDelayTimer.isActive()) {
        m_searchProcessingDelayTimer.stop();
    }
    m_cachedResults.clear();
    m_category_results.clear();
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

void Scope::setScopesInstance(Scopes* scopes)
{
    if (m_metadataConnection) {
        QObject::disconnect(m_metadataConnection);
    }

    m_scopesInstance = scopes;
    if (m_scopesInstance) {
        m_metadataConnection = QObject::connect(scopes, &Scopes::metadataRefreshed, this, &Scope::metadataRefreshed);
        m_locationService = m_scopesInstance->locationService();
        // TODO Notify the user the the location has changed
        // connect(m_locationService.data(), &LocationService::locationChanged, this, &Scope::invalidateResults);
    }
}

void Scope::setSearchInProgress(bool searchInProgress)
{
    if (m_searchInProgress != searchInProgress) {
        m_searchInProgress = searchInProgress;
        Q_EMIT searchInProgressChanged();
    }
}

void Scope::setActivationInProgress(bool activationInProgress)
{
    if (m_activationInProgress != activationInProgress) {
        m_activationInProgress = activationInProgress;
        Q_EMIT activationInProgressChanged();
    }
}

void Scope::setStatus(shell::scopes::ScopeInterface::Status status)
{
    if (m_status != status) {
        m_status = status;
        Q_EMIT statusChanged();
    }
}

void Scope::setCurrentNavigationId(QString const& id)
{
    if (m_currentNavigationId != id) {
        qDebug() << "Setting current nav id:" <<  this->id() << id;
        processPrimaryNavigationTag(id);
        m_currentNavigationId = id;
        Q_EMIT currentNavigationIdChanged();
    }
}

void Scope::setFilterState(scopes::FilterState const& filterState)
{
    m_filterState = filterState;
}

void Scope::dispatchSearch()
{
    m_initialQueryDone = true;

    invalidateLastSearch();
    m_delayedSearchProcessing = true;
    m_category_results.clear();
    m_categories->markNewSearch();

    m_searchProcessingDelayTimer.start(SEARCH_PROCESSING_DELAY);
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

    // If applicable, update this scope's child scopes now, as part of the search process
    // (i.e. while the loading bar is visible).
    update_child_scopes();

    if (m_proxy) {
        scopes::SearchMetadata meta(m_cardinality, QLocale::system().name().toStdString(), m_formFactor.toStdString());
        auto const userAgent = m_scopesInstance->userAgentString();
        if (!userAgent.isEmpty()) {
            meta["user-agent"] = userAgent.toStdString();
        }

        if (!m_session_id.isNull()) {
            meta["session-id"] = uuidToString(m_session_id).toStdString();
        }
        meta["query-id"] = unity::scopes::Variant(m_query_id);
        try {
            if (m_settingsModel && m_scopeMetadata && m_scopeMetadata->location_data_needed())
            {
                QVariant locationEnabled = m_settingsModel->value(QStringLiteral("internal.location"));
                if (locationEnabled.type() == QVariant::Bool && locationEnabled.toBool())
                {
                    meta.set_location(m_locationService->location());
                }
            }
        }
        catch (std::domain_error& e)
        {
        }
        meta.set_internet_connectivity(m_network_manager.isOnline() ? scopes::SearchMetadata::Connected : scopes::SearchMetadata::Disconnected);

        scopes::SearchListenerBase::SPtr listener(new SearchResultReceiver(this));
        m_searchController->setListener(listener);
        try {
            qDebug() << "Dispatching search:" << id() << m_searchQuery << m_currentNavigationId;
            scopes::QueryCtrlProxy controller = m_queryUserData ?
                m_proxy->search(m_searchQuery.toStdString(), m_currentNavigationId.toStdString(), m_filterState, *m_queryUserData, meta, listener) :
                m_proxy->search(m_searchQuery.toStdString(), m_currentNavigationId.toStdString(), m_filterState, meta, listener);
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

    QVariant converted(scopeVariantToQVariant(scopes::Variant(m_scopeMetadata->appearance_attributes())));
    m_customizations = converted.toMap();
    Q_EMIT customizationsChanged();

    try
    {
        scopes::Variant settings_definitions;
        settings_definitions = m_scopeMetadata->settings_definitions();
        QDir shareDir;
        if(qEnvironmentVariableIsSet("UNITY_SCOPES_CONFIG_DIR"))
        {
            shareDir = qgetenv("UNITY_SCOPES_CONFIG_DIR");
        }
        else
        {
            shareDir = QDir::home().filePath(QStringLiteral(".config/unity-scopes"));
        }

        m_settingsModel.reset(
                new SettingsModel(shareDir, id(),
                        scopeVariantToQVariant(settings_definitions), this));
        QObject::connect(m_settingsModel.data(), &SettingsModel::settingsChanged, this, &Scope::invalidateResults);
    }
    catch (unity::scopes::NotFoundException&)
    {
        // If there's no settings data
        m_settingsModel.reset();
    }
    Q_EMIT settingsChanged();
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
    std::string search_hint;
    try {
        if (m_scopeMetadata) {
            search_hint = m_scopeMetadata->search_hint();
        }
    } catch (...) {
        // throws if the value isn't set, safe to ignore
    }

    return QString::fromStdString(search_hint);
}

bool Scope::searchInProgress() const
{
    return m_searchInProgress;
}

bool Scope::activationInProgress() const
{
    return m_activationInProgress;
}

unity::shell::scopes::ScopeInterface::Status Scope::status() const
{
    return m_status;
}

bool Scope::favorite() const
{
    return m_favorite;
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
    return m_categories.data();
}

unity::shell::scopes::SettingsModelInterface* Scope::settings() const
{
    return m_settingsModel.data();
}

bool Scope::require_child_scopes_refresh() const
{
    if (m_settingsModel && m_scopesInstance)
    {
        return m_settingsModel->require_child_scopes_refresh();
    }
    return false;
}

void Scope::update_child_scopes()
{
    // only run the update if child scopes have changed
    if (m_childScopesDirty && m_settingsModel && m_scopesInstance)
    {
        // reset the flag so that re-entering this method won't restart the update unnecessarily
        m_childScopesDirty = false;

        // just in case we have another update still running, wait here for it to complete
        m_childScopesFuture.waitForFinished();

        // run the update in a seperate thread
        m_childScopesFuture = QtConcurrent::run([this]
        {
            m_settingsModel->update_child_scopes(m_scopesInstance->getAllMetadata());
        });
    }
}

unity::shell::scopes::NavigationInterface* Scope::getNavigation(QString const& navId)
{
    if (!m_departmentTree) return nullptr;

    DepartmentNode* node = m_departmentTree->findNodeById(navId);
    if (!node) return nullptr;

    Department* navModel = new Department;
    navModel->setScopeId(this->id());
    navModel->loadFromDepartmentNode(node);
    navModel->markSubdepartmentActive(m_currentNavigationId);

    // sharing m_inverseDepartments with getAltNavigation
    m_departmentModels.insert(navId, navModel);
    m_inverseDepartments.insert(navModel, navId);
    QObject::connect(navModel, &QObject::destroyed, this, &Scope::departmentModelDestroyed);

    return navModel;
}

QString Scope::buildQuery(QString const& scopeId, QString const& searchQuery, QString const& departmentId, unity::scopes::FilterState const& filterState)
{
    scopes::CannedQuery q(scopeId.toStdString());
    q.set_query_string(searchQuery.toStdString());
    q.set_department_id(departmentId.toStdString());
    q.set_filter_state(filterState);
    return QString::fromStdString(q.to_uri());
}

void Scope::setNavigationState(QString const& navId)
{
    // switch current department id
    performQuery(buildQuery(id(), m_searchQuery, navId, m_filterState));
}

void Scope::departmentModelDestroyed(QObject* obj)
{
    scopes_ng::Department* navigation = reinterpret_cast<scopes_ng::Department*>(obj);

    auto it = m_inverseDepartments.find(navigation);
    if (it == m_inverseDepartments.end()) return;

    m_departmentModels.remove(it.value(), navigation);
    m_inverseDepartments.erase(it);
}

void Scope::previewModelDestroyed(QObject *obj)
{
    for (auto it = m_previewModels.begin(); it != m_previewModels.end(); it++)
    {
        if (*it == obj) {
            qDebug() << "PreviewModel destroyed";
            m_previewModels.erase(it);
            break;
        }
    }
}

void Scope::performQuery(QString const& cannedQuery)
{
    try {
        scopes::CannedQuery q(scopes::CannedQuery::from_uri(cannedQuery.toStdString()));
        executeCannedQuery(q, true);
    } catch (...) {
        qWarning("Unable to parse canned query uri: %s", cannedQuery.toStdString().c_str());
    }
}

void Scope::refresh()
{
    // shell has to specifically call this, maybe we should ignore the active flag here and just call dispatchSearch
    invalidateResults();
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

QString Scope::currentNavigationId() const
{
    return m_currentNavigationId;
}

bool Scope::hasNavigation() const
{
    return m_hasNavigation;
}

QVariantMap Scope::customizations() const
{
    return m_customizations;
}

int Scope::activeFiltersCount() const
{
    return m_activeFiltersCount;
}

void Scope::setSearchQuery(const QString& search_query)
{
    // this method is called by the shell when user types in search string,
    // it needs to reset canned query user data.
    if (m_searchQuery.isNull() || search_query != m_searchQuery) {
        m_queryUserData.reset(nullptr);
    }
    setSearchQueryString(search_query);
}

void Scope::setSearchQueryString(const QString& search_query)
{
    /* Checking for m_searchQuery.isNull() which returns true only when the string
       has never been set is necessary because when search_query is the empty
       string ("") and m_searchQuery is the null string,
       search_query != m_searchQuery is still true.
    */

    if (m_searchQuery.isNull() || search_query != m_searchQuery) {
        // regenerate session id uuid if previous or current search string is empty or
        // if current and previous query have no common prefix;
        // don't regenerate it if current query appends to previous query or removes
        // characters from previous query.
        bool search_empty = m_searchQuery.isEmpty() || search_query.isEmpty();

        // only check for common prefix if search is not empty
        bool common_prefix = (!search_empty) && (m_searchQuery.startsWith(search_query) || search_query.startsWith(m_searchQuery));

        if (m_session_id.isNull() || search_empty || !common_prefix) {
            m_session_id = QUuid::createUuid();
            m_query_id = 0;
        } else {
            ++m_query_id;
        }
        m_searchQuery = search_query;

        // only use typing delay if scope is active, otherwise apply immediately
        if (m_isActive) {
            m_typingTimer.start();
        } else {
            invalidateResults();
            Q_EMIT searchQueryChanged();
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
        Q_EMIT isActiveChanged();

        if (m_scopeMetadata && m_scopeMetadata->location_data_needed())
        {
            if (m_isActive)
            {
                m_locationToken = m_locationService->activate();
            }
            else
            {
                m_locationToken.reset();
            }
        }

        if (active && m_resultsDirty) {
            dispatchSearch();
        }
    }
}

void Scope::setFavorite(const bool value)
{
    if (value != m_favorite)
    {
        m_favorite = value;
        Q_EMIT favoriteChanged(value);
        m_scopesInstance->setFavorite(id(), value);
    }
}

void Scope::activate(QVariant const& result_var, QString const& categoryId)
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

    auto const activateResult = [this, result, categoryId, result_var]() {
        if (result->direct_activation()) {
            if (result->uri().find("scope://") == 0 || id() == QLatin1String("clickscope") || (id() == QLatin1String("videoaggregator") && categoryId == QLatin1String("myvideos-getstarted"))) {
                activateUri(QString::fromStdString(result->uri()));
            } else {
                Q_EMIT previewRequested(result_var);
            }
        } else { // intercept activation flag set
            try {
                cancelActivation();
                scopes::ActivationListenerBase::SPtr listener(new ActivationReceiver(this, result));
                m_activationController->setListener(listener);

                setActivationInProgress(true);

                auto proxy = proxy_for_result(result);
                unity::scopes::ActionMetadata metadata(QLocale::system().name().toStdString(), m_formFactor.toStdString());
                scopes::QueryCtrlProxy controller = proxy->activate(*(result.get()), metadata, listener);
                m_activationController->setController(controller);
            } catch (std::exception& e) {
                setActivationInProgress(false);
                qWarning("Caught an error from activate(): %s", e.what());
            } catch (...) {
                setActivationInProgress(false);
                qWarning("Caught an error from activate()");
            }
        }
    };

    if (result->contains("online_account_details"))
    {
        QVariantMap details = scopeVariantToQVariant(result->value("online_account_details")).toMap();
        if (details.contains(QStringLiteral("service_name")) &&
            details.contains(QStringLiteral("service_type")) &&
            details.contains(QStringLiteral("provider_name")) &&
            details.contains(QStringLiteral("login_passed_action")) &&
            details.contains(QStringLiteral("login_failed_action")))
        {
            LoginToAccount *login = new LoginToAccount(details.contains(QStringLiteral("scope_id")) ? details.value(QStringLiteral("scope_id")).toString() : id(),
                                                       details.value(QStringLiteral("service_name")).toString(),
                                                       details.value(QStringLiteral("service_type")).toString(),
                                                       details.value(QStringLiteral("provider_name")).toString(),
                                                       details.value(QStringLiteral("auth_params")).toMap(),
                                                       details.value(QStringLiteral("login_passed_action")).toInt(),
                                                       details.value(QStringLiteral("login_failed_action")).toInt(),
                                                       this);
            connect(login, SIGNAL(searchInProgress(bool)), this, SLOT(setSearchInProgress(bool)));
            connect(login, &LoginToAccount::finished, [this, login, activateResult](bool, int action_code_index) {
                if (action_code_index >= 0 && action_code_index <= scopes::OnlineAccountClient::LastActionCode_)
                {
                    scopes::OnlineAccountClient::PostLoginAction action_code = static_cast<scopes::OnlineAccountClient::PostLoginAction>(action_code_index);
                    switch (action_code)
                    {
                        case scopes::OnlineAccountClient::DoNothing:
                            return;
                        case scopes::OnlineAccountClient::InvalidateResults:
                            invalidateResults();
                            return;
                        default:
                            break;
                    }
                }
                activateResult();
                login->deleteLater();
            });
            login->loginToAccount();
            return; // main exectuion ends here
        }
    } else {
        activateResult();
    }
}

// called for in-card (result) actions.
void Scope::activateAction(QVariant const& result_var, QString const& categoryId, QString const& actionId)
{
    try {
        cancelActivation();
        std::shared_ptr<scopes::Result> result = result_var.value<std::shared_ptr<scopes::Result>>();
        scopes::ActivationListenerBase::SPtr listener(new ActivationReceiver(this, result, categoryId));
        m_activationController->setListener(listener);

        qDebug() << "Activating result action for result with uri '" << QString::fromStdString(result->uri()) << ", categoryId" << categoryId;

        auto proxy = proxy_for_result(result);
        unity::scopes::ActionMetadata metadata(QLocale::system().name().toStdString(), m_formFactor.toStdString());
        scopes::QueryCtrlProxy controller = proxy->activate_result_action(*(result.get()), metadata, actionId.toStdString(), listener);
        m_activationController->setController(controller);
    } catch (std::exception& e) {
        qWarning("Caught an error from activate_result_action(): %s", e.what());
    } catch (...) {
        qWarning("Caught an error from activate_result_action()");
    }
}

unity::shell::scopes::PreviewModelInterface* Scope::preview(QVariant const& result_var, QString const& categoryId)
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

    // No preview for scope:// uris and for special camera-app card in video aggregator scope (if no videos are available).
    if (result->uri().find("scope://") == 0 || (id() == QLatin1String("videoaggregator") && categoryId == QLatin1String("myvideos-getstarted"))) {
        return nullptr;
    }

    PreviewModel* previewModel = new PreviewModel(nullptr);
    QObject::connect(previewModel, &QObject::destroyed, this, &Scope::previewModelDestroyed);
    m_previewModels.append(previewModel);
    previewModel->setAssociatedScope(this, m_session_id, m_scopesInstance->userAgentString());
    previewModel->loadForResult(result);
    return previewModel;
}

void Scope::cancelActivation()
{
    m_activationController->invalidate();
}

void Scope::invalidateChildScopes()
{
    m_childScopesDirty = true;
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

void Scope::resetPrimaryNavigationTag()
{
    qDebug() << "resetPrimaryNavigationTag()";
    setCurrentNavigationId("");
    m_filters->update(unity::scopes::FilterState());
    filterStateChanged();
}

void Scope::resetFilters()
{
    m_filters->reset();
}

void Scope::closeScope(unity::shell::scopes::ScopeInterface* scope)
{
    if (m_scopesInstance) {
        m_scopesInstance->closeScope(scope);
    }
}

bool Scope::resultsDirty() const {
    return m_resultsDirty;
}

QString Scope::sessionId() const {
    return uuidToString(m_session_id);
}

int Scope::queryId() const {
    return m_query_id;
}

void Scope::activateUri(QString const& uri)
{
    /*
     * If it's a scope URI, perform the query, otherwise ask Qt to open it.
     */
    Q_EMIT gotoUri(uri);
    QUrl url(uri);
    if (url.scheme() == QLatin1String("scope"))
    {
        performQuery(uri);
    }
    else
    {
        if (qEnvironmentVariableIsEmpty("UNITY_SCOPES_NO_OPEN_URL"))
        {
            QDesktopServices::openUrl(url);
        }
    }
}

bool Scope::initialQueryDone() const
{
    return m_initialQueryDone;
}

unity::shell::scopes::FiltersInterface* Scope::filters() const
{
    if (m_filters && m_filters->rowCount() == 0) {
        return nullptr;
    }
    return m_filters.data();
}

unity::shell::scopes::FilterBaseInterface* Scope::primaryNavigationFilter() const
{
    return m_filters->primaryFilter().data();
}

QString Scope::primaryNavigationTag() const
{
    return m_primaryNavigationTag;
}

void Scope::filterStateChanged()
{
    qDebug() << "Filters changed";
    m_filterState = m_filters->filterState();
    processPrimaryNavigationTag(m_currentNavigationId);
    processActiveFiltersCount();
    invalidateResults();
}

//
// Iterate over all filters to calculate the number of active ones.
void Scope::processActiveFiltersCount()
{
    const int count = m_filters->activeFiltersCount();
    if (count != m_activeFiltersCount) {
        m_activeFiltersCount = count;
        Q_EMIT activeFiltersCountChanged();
    }
    qDebug() << "active filters count:" << m_activeFiltersCount;
}

//
// Determine primary navigation tag (the "brick" in search bar) from
// current department (if departments are present) or primary navigation
// filter (if scopes doesn't have departments but has filters and one of
// them has 'Primary' flag set.
void Scope::processPrimaryNavigationTag(QString const &targetDepartmentId)
{
    QString tag;
    // has departments?
    if (m_rootDepartment) {
        auto it = m_departmentModels.constFind(targetDepartmentId);
        if (it != m_departmentModels.constEnd()) {
            tag = (targetDepartmentId == "" ? "" : it.value()->label());
        } else {
            it = m_departmentModels.constFind(m_currentNavigationId);
            if (it != m_departmentModels.constEnd()) {
                auto subDept = (*it)->findSubdepartment(targetDepartmentId);
                if (subDept) {
                    tag = subDept->label;
                } else {
                    qWarning() << "Scope::processPrimaryNavigationTag(): no subdepartment '" << targetDepartmentId << "'";
                }
            } else {
                qWarning() << "Scope::processPrimaryNavigationTag(): no department model for '" << m_currentNavigationId << "'";
            }
        }
    } else {
        auto pf = m_filters->primaryFilter();
        if (pf) {
            tag = pf->filterTag();
        }
    }
    qDebug() << "Scope::processPrimaryNavigationTag(): tag is '" << tag << "'";
    if (m_primaryNavigationTag != tag) {
        m_primaryNavigationTag = tag;
        Q_EMIT primaryNavigationTagChanged();
    }
}

} // namespace scopes_ng
