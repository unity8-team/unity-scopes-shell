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

#include <memory>
#include <string>

#include <QVariantMap>
#include <qglobal.h>

#include <unity/shell/scopes/ScopeInterface.h>

#include <unity/util/DefinesPtrs.h>

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

class Q_DECL_EXPORT ResultsView
{
public:
    UNITY_DEFINES_PTRS(ResultsView);

    ResultsView(std::shared_ptr<scopes_ng::Scopes> scopes);

    ~ResultsView() = default;

    void setQuery(const std::string& searchString);

    void setActiveScope(const std::string& id);

    void waitForResultsChange();

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

    unity::shell::scopes::ScopeInterface::Status status() const;

    // TODO Remove / replace these
    unity::shell::scopes::CategoriesInterface* categories();

    unity::shell::scopes::ScopeInterface* activeScope();

protected:
    struct Priv;

    std::shared_ptr<Priv> p;
};

}
}
