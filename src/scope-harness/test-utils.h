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

#include <Unity/scope.h>

#include <unity/scopes/Result.h>

#include <QScopedPointer>
#include <QStringList>

namespace unity {
namespace scopeharness {

static constexpr int SIG_SPY_TIMEOUT = 60000; // milliseconds

class TestUtils
{
public:

Q_DECL_EXPORT
static void throwIf(bool condition, const std::string& message);

Q_DECL_EXPORT
static void throwIfNot(bool condition, const std::string& message);

Q_DECL_EXPORT
static void checkedFirstResult(unity::shell::scopes::CategoriesInterface* categories, unity::scopes::Result::SPtr& result, bool& success);

Q_DECL_EXPORT
static bool getFirstResult(unity::shell::scopes::CategoriesInterface* categories, unity::scopes::Result::SPtr& result);

Q_DECL_EXPORT
static void refreshSearch(scopes_ng::Scope::Ptr);

Q_DECL_EXPORT
static void performSearch(QSharedPointer<shell::scopes::ScopeInterface> scope, QString const& searchString);

Q_DECL_EXPORT
static void waitForFilterStateChange(QSharedPointer<shell::scopes::ScopeInterface> scope);

Q_DECL_EXPORT
static void waitForResultsChange(QSharedPointer<shell::scopes::ScopeInterface> scope);

Q_DECL_EXPORT
static void waitForSearchFinish(QSharedPointer<shell::scopes::ScopeInterface> scope);

Q_DECL_EXPORT
static void setFavouriteScopes(const QStringList& cannedQueries);

Q_DECL_EXPORT
static QStringList getFavoriteScopes();

};


}
}
