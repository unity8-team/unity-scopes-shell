/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
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
 *
 * Authors:
 *  Michal Hruby <michal.hruby@canonical.com>
 */

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <scope.h>

#include <unity/scopes/Result.h>

#include <QScopedPointer>
#include <QStringList>

namespace scopes_ng
{

Q_DECL_EXPORT
void checkedFirstResult(Scope* scope, unity::scopes::Result::SPtr& result, bool& success);

Q_DECL_EXPORT
bool getFirstResult(Scope* scope, unity::scopes::Result::SPtr& result);

Q_DECL_EXPORT
void refreshSearch(scopes_ng::Scope*);

Q_DECL_EXPORT
void performSearch(Scope* scope, QString const& searchString);

Q_DECL_EXPORT
void waitForResultsChange(Scope* scope);

Q_DECL_EXPORT
bool previewForFirstResult(Scope* scope, QString const& searchString, QScopedPointer<PreviewStack>& preview_stack);

Q_DECL_EXPORT
void setFavouriteScopes(const QStringList& cannedQueries);

Q_DECL_EXPORT
QStringList getFavoriteScopes();

}

#endif //TEST_UTILS_H
