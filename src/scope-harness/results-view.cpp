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

#include <scope-harness/internal/category-arguments.h>
#include <scope-harness/internal/result-arguments.h>
#include <scope-harness/preview-view.h>
#include <scope-harness/results-view.h>
#include <scope-harness/test-utils.h>

using namespace std;
namespace ng = scopes_ng;
namespace sc = unity::scopes;
namespace ss = unity::shell::scopes;

namespace unity
{
namespace scopeharness
{

struct ResultsView::Priv
{
    void checkActiveScope()
    {
        throwIfNot(m_active_scope, "There is no active scope");
    }

    std::shared_ptr<ng::Scopes> m_scopes;

    ng::Scope* m_active_scope = nullptr;
};

ResultsView::ResultsView(std::shared_ptr<ng::Scopes> scopes) :
        p(new Priv)
{
    p->m_scopes = scopes;
}

void ResultsView::setActiveScope(const std::string &id_)
{
    QString id = QString::fromStdString(id_);

    // Deactivate the previous scopes first
    for (int row = 0; row < p->m_scopes->rowCount(); ++row)
    {
        ng::Scope* scope = static_cast<ng::Scope*>(p->m_scopes->getScopeByRow(row));
        if (scope->id() != id)
        {
            scope->setActive(false);
        }
    }

    p->m_active_scope = nullptr;

    // Activate the new scope
    for (int row = 0; row < p->m_scopes->rowCount(); ++row)
    {
        ng::Scope* scope = static_cast<ng::Scope*>(p->m_scopes->getScopeByRow(row));
        if (scope->id() == id)
        {
            p->m_active_scope = scope;
            QSignalSpy spy(scope, SIGNAL(searchInProgressChanged()));

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

void ResultsView::setQuery(const std::string& searchString_)
{
    p->checkActiveScope();

    QString searchString = QString::fromStdString(searchString_);

    // If we've already got a completed search for this string then do nothing
    if (p->m_active_scope->searchQuery() == searchString)
    {
        return;
    }

    throwIf(p->m_active_scope->searchInProgress(), "Search is already in progress");

    QSignalSpy spy(p->m_active_scope, SIGNAL(searchInProgressChanged()));
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

void ResultsView::waitForResultsChange()
{
    p->checkActiveScope();

    throwIf(p->m_active_scope->searchInProgress(), "Search is already in progress");
    // wait for the search to finish
    QSignalSpy spy(p->m_active_scope, SIGNAL(searchInProgressChanged()));
    throwIfNot(spy.wait(), "Search status didn't change");
    if(spy.size() == 1) {
        throwIfNot(spy.wait(), "Search status didn't change");
    }
    throwIf(p->m_active_scope->searchInProgress(), "");
}

bool ResultsView::overrideCategoryJson(std::string const& categoryId, std::string const& json)
{
    p->checkActiveScope();

    return p->m_active_scope->categories()->overrideCategoryJson(
            QString::fromStdString(categoryId), QString::fromStdString(json));
}

Category::List ResultsView::categories() const
{
    auto cats = raw_categories();

    Category::List result;
    for (int i = 0; i < cats->rowCount(); ++i)
    {
        try
        {
            result.emplace_back(category(i));
        }
        catch (range_error& e)
        {
        }
    }

    return result;
}

Category ResultsView::category(unsigned int row) const
{
    qDebug() << "FOO !!!!!! 1";

    auto cats = raw_categories();
    auto categoryIndex = cats->index(row);
    qDebug() << "FOO !!!!!! 2";

    QVariant variant = cats->data(categoryIndex, 999999);
    if (!variant.canConvert<sc::Category::SCPtr>())
    {
        throw std::range_error("Invalid category data at index " + to_string(row));
    }
    qDebug() << "FOO !!!!!! 3";
    auto rawCategory = variant.value<sc::Category::SCPtr>();
    qDebug() << "FOO !!!!!! 4";

    QVariant resultsVariant = cats->data(
            cats->index(row), ss::CategoriesInterface::Roles::RoleResults);
    ss::ResultsModelInterface* resultModel = resultsVariant.value<
            ss::ResultsModelInterface*>();
    qDebug() << "FOO !!!!!! 5";
    Result::List results;
    if (resultModel)
    {
        qDebug() << "FOO !!!!!! 6";
        for (int i = 0; i < resultModel->rowCount(); ++i)
        {
            qDebug() << "FOO !!!!!! 7";
            auto idx = resultModel->index(i);
            results.emplace_back(Result(internal::ResultArguments{resultModel, p->m_active_scope, idx, PreviewView::SPtr()}));
        }
    }
    qDebug() << "FOO !!!!!! 8";

    return Category(internal::CategoryArguments{cats, categoryIndex, results});
}

Category ResultsView::category(const string& categoryId_) const
{
    qDebug() << "HELLO !!!!!! 1";

    auto cats = raw_categories();

    QString categoryId = QString::fromStdString(categoryId_);
    int row = -1;
    qDebug() << "HELLO !!!!!! 2";

    for (int i = 0; row < cats->rowCount(); ++row)
    {
        qDebug() << "HELLO !!!!!! 3";
        QVariant variant = cats->data(cats->index(i),
                                      ss::CategoriesInterface::RoleCategoryId);
        if (variant.toString() == categoryId)
        {
            qDebug() << "HELLO !!!!!! 4";
            row = i;
            break;
        }
    }

    qDebug() << "HELLO !!!!!! 5";
    throwIf(row == -1, "Could not find category");
    qDebug() << "HELLO !!!!!! 6";
    return category(row);
}

ss::CategoriesInterface* ResultsView::raw_categories() const
{
    p->checkActiveScope();
    return p->m_active_scope->categories();
}

ss::ScopeInterface* ResultsView::activeScope() const
{
    p->checkActiveScope();
    return p->m_active_scope;
}

std::string ResultsView::scopeId() const
{
    p->checkActiveScope();
    return p->m_active_scope->id().toStdString();
}

std::string ResultsView::displayName() const
{
    p->checkActiveScope();
    return p->m_active_scope->name().toStdString();
}

std::string ResultsView::iconHint() const
{
    p->checkActiveScope();
    return p->m_active_scope->iconHint().toStdString();
}

std::string ResultsView::description() const
{
    p->checkActiveScope();
    return p->m_active_scope->description().toStdString();
}

std::string ResultsView::searchHint() const
{
    p->checkActiveScope();
    return p->m_active_scope->searchHint().toStdString();
}

std::string ResultsView::shortcut() const
{
    p->checkActiveScope();
    return p->m_active_scope->shortcut().toStdString();
}

std::string ResultsView::searchQuery() const
{
    p->checkActiveScope();
    return p->m_active_scope->searchQuery().toStdString();
}

QVariantMap ResultsView::customizations() const
{
    p->checkActiveScope();
    return p->m_active_scope->customizations();
}

std::string ResultsView::sessionId() const
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
