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
namespace internal {

Q_DECL_EXPORT
void throwIf(bool condition, const std::string& message);

Q_DECL_EXPORT
void throwIfNot(bool condition, const std::string& message);

Q_DECL_EXPORT
void checkedFirstResult(unity::shell::scopes::CategoriesInterface* categories, unity::scopes::Result::SPtr& result, bool& success);

Q_DECL_EXPORT
bool getFirstResult(unity::shell::scopes::CategoriesInterface* categories, unity::scopes::Result::SPtr& result);

Q_DECL_EXPORT
void refreshSearch(scopes_ng::Scope::Ptr);

Q_DECL_EXPORT
void performSearch(QSharedPointer<shell::scopes::ScopeInterface> scope, QString const& searchString);

Q_DECL_EXPORT
void waitForResultsChange(QSharedPointer<shell::scopes::ScopeInterface> scope);

Q_DECL_EXPORT
void waitForSearchFinish(QSharedPointer<shell::scopes::ScopeInterface> scope);

Q_DECL_EXPORT
bool previewForFirstResult(scopes_ng::Scope::Ptr scope, QString const& searchString, QScopedPointer<scopes_ng::PreviewStack>& preview_stack);

Q_DECL_EXPORT
void setFavouriteScopes(const QStringList& cannedQueries);

Q_DECL_EXPORT
QStringList getFavoriteScopes();

}
}
}
