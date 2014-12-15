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

#pragma once

#include <unity/shell/scopes/ScopeInterface.h>

#include <scope-harness/preview-view.h>
#include <scope-harness/category.h>

#include <memory>
#include <string>

#include <QVariantMap>
#include <qglobal.h>

namespace scopes_ng
{
class Scopes;
}

namespace unity
{
namespace shell
{
namespace scopes
{
class CategoriesInterface;
}
}

namespace scopeharness
{

class Q_DECL_EXPORT ResultsView: public AbstractView
{
public:
    UNITY_DEFINES_PTRS(ResultsView);

    ResultsView(std::shared_ptr<scopes_ng::Scopes> scopes, PreviewView::SPtr previewView);

    ~ResultsView() = default;

    void setQuery(const std::string& searchString);

    void setActiveScope(const std::string& id);

    void waitForResultsChange();

    bool overrideCategoryJson(std::string const& categoryId, std::string const& json);

    std::string scopeId() const;

    std::string displayName() const;

    std::string iconHint() const;

    std::string description() const;

    std::string searchHint() const;

    std::string shortcut() const;

    std::string searchQuery() const;

    QVariantMap customizations() const;

    std::string sessionId() const;

    int queryId() const;

    Category::List categories();

    Category category(unsigned int row);

    Category category(const std::string& categoryId);

    unity::shell::scopes::ScopeInterface::Status status() const;

    // TODO Remove / replace these
    unity::shell::scopes::CategoriesInterface* raw_categories() const;

    QSharedPointer<unity::shell::scopes::ScopeInterface> activeScope() const;

protected:
    struct Priv;

    std::shared_ptr<Priv> p;
};

}
}
