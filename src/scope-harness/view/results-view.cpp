/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <QSignalSpy>

#include <Unity/scopes.h>
#include <Unity/categories.h>
#include <Unity/utils.h>

#include <scope-harness/internal/category-arguments.h>
#include <scope-harness/internal/department-arguments.h>
#include <scope-harness/internal/result-arguments.h>
#include <scope-harness/internal/results-view-arguments.h>
#include <scope-harness/view/preview-view.h>
#include <scope-harness/view/results-view.h>
#include <scope-harness/internal/test-utils.h>

using namespace std;
namespace ng = scopes_ng;
namespace sc = unity::scopes;
namespace ss = unity::shell::scopes;

namespace unity
{
namespace scopeharness
{
using namespace internal;
namespace view
{

struct ResultsView::Priv
{
    Priv(ResultsView& self, shared_ptr<ng::Scopes> scopes) :
        m_self(self), m_scopes(scopes)
    {
    }

    void checkActiveScope() const
    {
        throwIfNot(m_active_scope, "There is no active scope");
    }

    ss::CategoriesInterface* internalRawCategories() const
    {
        checkActiveScope();
        return m_active_scope->categories();
    }

    results::Category internalCategory(size_t row)
    {
        auto cats = internalRawCategories();
        auto categoryIndex = cats->index(row);

        QVariant variant = cats->data(categoryIndex, ng::Categories::RoleCategorySPtr);
        if (!variant.canConvert<sc::Category::SCPtr>())
        {
            throw range_error("Invalid category data at index " + to_string(row));
        }
        auto rawCategory = variant.value<sc::Category::SCPtr>();

        QVariant resultsVariant = cats->data(
                cats->index(row), ng::Categories::RoleResultsSPtr);
        QSharedPointer<ss::ResultsModelInterface> resultModel = resultsVariant.value<
                QSharedPointer<ss::ResultsModelInterface>>();
        results::Result::List results;
        if (resultModel)
        {
            for (int i = 0; i < resultModel->rowCount(); ++i)
            {
                auto idx = resultModel->index(i);
                results.emplace_back(results::Result(internal::ResultArguments
                    { resultModel, m_active_scope, idx,
                      dynamic_pointer_cast<ResultsView>(m_self.shared_from_this()),
                      m_previewView.lock() }));
            }
        }

        return results::Category(internal::CategoryArguments{cats, categoryIndex, results});
    }

    results::Department browseDepartment(const string& id, bool altNavigation)
    {
        checkActiveScope();

        QSharedPointer<ss::NavigationInterface> navigationModel;
        if (altNavigation)
        {
            navigationModel.reset(m_active_scope->getAltNavigation(QString::fromStdString(id)));
        }
        else
        {
            navigationModel.reset(m_active_scope->getNavigation(QString::fromStdString(id)));
        }
        throwIfNot(bool(navigationModel), "Unknown department: '" + id + "'");

        QSignalSpy spy(navigationModel.data(), SIGNAL(loadedChanged()));

        bool shouldUpdate = false;

        if (altNavigation)
        {
            if (m_active_scope->currentAltNavigationId().toStdString() != id)
            {
                shouldUpdate = true;
            }
        }
        else
        {
            if (m_active_scope->currentNavigationId().toStdString() != id)
            {
                shouldUpdate = true;
            }
        }

        if (shouldUpdate)
        {
            m_active_scope->setNavigationState(QString::fromStdString(id),
                                               altNavigation);
            waitForSearchFinish(m_active_scope);
        }

        if (!navigationModel->loaded())
        {
            throwIfNot(spy.wait(), "Department model failed to load");
        }

        return results::Department(internal::DepartmentArguments{navigationModel});
    }

    ResultsView& m_self;

    shared_ptr<ng::Scopes> m_scopes;

    weak_ptr<PreviewView> m_previewView;

    ng::Scope::Ptr m_active_scope;
};

ResultsView::ResultsView(const internal::ResultsViewArguments& arguments) :
        p(new Priv(*this, arguments.scopes))
{
}

void ResultsView::setPreviewView(PreviewView::SPtr previewView)
{
    p->m_previewView = previewView;
}

void ResultsView::setActiveScope(const string &id_)
{
    QString id = QString::fromStdString(id_);

    // Deactivate the previous scopes first
    for (int row = 0; row < p->m_scopes->rowCount(); ++row)
    {
        ng::Scope::Ptr scope = p->m_scopes->getScopeByRow(row);
        if (scope->id() != id)
        {
            scope->setActive(false);
        }
    }

    p->m_active_scope.reset();

    // Activate the new scope
    for (int row = 0; row < p->m_scopes->rowCount(); ++row)
    {
        ng::Scope::Ptr scope = p->m_scopes->getScopeByRow(row);
        if (scope->id() == id)
        {
            p->m_active_scope = scope;
            QSignalSpy spy(scope.data(), SIGNAL(searchInProgressChanged()));

            scope->setSearchQuery("");
            scope->setActive(true);

            if (!scope->searchInProgress())
            {
                spy.wait(100);
            }
            if (scope->searchInProgress())
            {
                throwIfNot(spy.wait(), "Active scope didn't finish searching");
            }

            break;
        }
    }
}

string ResultsView::activeScope() const
{
    string result;

    if (p->m_active_scope)
    {
        result = p->m_active_scope->id().toStdString();
    }

    return result;
}

void ResultsView::setQuery(const string& searchString_)
{
    p->checkActiveScope();

    QString searchString = QString::fromStdString(searchString_);

    // If we've already got a completed search for this string then do nothing
    if (p->m_active_scope->searchQuery() == searchString)
    {
        return;
    }

    throwIf(p->m_active_scope->searchInProgress(), "Search is already in progress");

    QSignalSpy spy(p->m_active_scope.data(), SIGNAL(searchInProgressChanged()));
    // perform a search
    p->m_active_scope->setSearchQuery(searchString);
    // search should not be happening yet
    throwIf(p->m_active_scope->searchInProgress(), "Search was in progress too soon");
    throwIfNot(spy.wait(), "Search spy received no events");
    if (p->m_active_scope->searchInProgress())
    {
        // wait for the search to finish
        throwIfNot(spy.wait(), "Search spy received no events");
    }
    throwIf(p->m_active_scope->searchInProgress(), "Search did not complete");
}

void ResultsView::forceRefresh()
{
    p->checkActiveScope();

    throwIf(p->m_active_scope->searchInProgress(), "Search is already in progress");

    QSignalSpy spy(p->m_active_scope.data(), SIGNAL(searchInProgressChanged()));
    // perform a search
    p->m_active_scope->refresh();
    // search should not be happening yet
    throwIfNot(p->m_active_scope->searchInProgress() || spy.count() > 1, "Refresh failed to start");
    if (p->m_active_scope->searchInProgress())
    {
        // wait for the search to finish
        throwIfNot(spy.wait(), "Search spy received no events");
    }
    throwIf(p->m_active_scope->searchInProgress(), "Search did not complete");
}

void ResultsView::waitForResultsChange()
{
    p->checkActiveScope();

    throwIf(p->m_active_scope->searchInProgress(), "Search is already in progress");
    // wait for the search to finish
    QSignalSpy spy(p->m_active_scope.data(), SIGNAL(searchInProgressChanged()));
    throwIfNot(spy.wait(), "Search status didn't change");
    if(spy.size() == 1) {
        throwIfNot(spy.wait(), "Search status didn't change");
    }
    throwIf(p->m_active_scope->searchInProgress(), "");
}

bool ResultsView::hasDepartments() const
{
    p->checkActiveScope();

    return p->m_active_scope->hasNavigation();
}

bool ResultsView::hasAltDepartments() const
{
    p->checkActiveScope();

    return p->m_active_scope->hasAltNavigation();
}

string ResultsView::departmentId() const
{
    p->checkActiveScope();

    return p->m_active_scope->currentNavigationId().toStdString();
}

string ResultsView::altDepartmentId() const
{
    p->checkActiveScope();

    return p->m_active_scope->currentAltNavigationId().toStdString();
}

results::Department ResultsView::browseDepartment(const string& id)
{
    return p->browseDepartment(id, false);
}

results::Department ResultsView::browseAltDepartment(const string& id)
{
    return p->browseDepartment(id, true);
}

bool ResultsView::overrideCategoryJson(string const& categoryId, string const& json)
{
    p->checkActiveScope();

    return p->m_active_scope->categories()->overrideCategoryJson(
            QString::fromStdString(categoryId), QString::fromStdString(json));
}

results::Category::List ResultsView::categories()
{
    auto cats = p->internalRawCategories();

    results::Category::List result;
    for (int i = 0; i < cats->rowCount(); ++i)
    {
        try
        {
            auto cat = p->internalCategory(i);
            if (!cat.empty())
            {
                result.emplace_back(cat);
            }
        }
        catch (range_error& e)
        {
        }
    }

    return result;
}

results::Category ResultsView::category(size_t row)
{
    auto cats = categories();
    return cats.at(row);
}

results::Category ResultsView::category(const string& categoryId_)
{
    auto cats = p->internalRawCategories();

    QString categoryId = QString::fromStdString(categoryId_);
    int row = -1;

    for (int i = 0; i < cats->rowCount(); ++i)
    {
        QVariant variant = cats->data(cats->index(i),
                                      ss::CategoriesInterface::RoleCategoryId);
        if (variant.toString() == categoryId)
        {
            row = i;
            break;
        }
    }

    throwIf(row == -1, "Could not find category");
    return p->internalCategory(row);
}

string ResultsView::scopeId() const
{
    p->checkActiveScope();
    return p->m_active_scope->id().toStdString();
}

string ResultsView::displayName() const
{
    p->checkActiveScope();
    return p->m_active_scope->name().toStdString();
}

string ResultsView::iconHint() const
{
    p->checkActiveScope();
    return p->m_active_scope->iconHint().toStdString();
}

string ResultsView::description() const
{
    p->checkActiveScope();
    return p->m_active_scope->description().toStdString();
}

string ResultsView::searchHint() const
{
    p->checkActiveScope();
    return p->m_active_scope->searchHint().toStdString();
}

string ResultsView::shortcut() const
{
    p->checkActiveScope();
    return p->m_active_scope->shortcut().toStdString();
}

string ResultsView::query() const
{
    p->checkActiveScope();
    return p->m_active_scope->searchQuery().toStdString();
}

sc::Variant ResultsView::customizations() const
{
    p->checkActiveScope();
    return ng::qVariantToScopeVariant(p->m_active_scope->customizations());
}

string ResultsView::sessionId() const
{
    p->checkActiveScope();
    return p->m_active_scope->sessionId().toStdString();
}

int ResultsView::queryId() const
{
    p->checkActiveScope();
    return p->m_active_scope->queryId();
}

unity::shell::scopes::ScopeInterface::Status ResultsView::status() const
{
    p->checkActiveScope();
    return p->m_active_scope->status();
}

}
}
}
