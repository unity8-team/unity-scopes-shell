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

#include <QObject>
#include <QTest>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QProcess>
#include <QThread>
#include <QScopedPointer>
#include <QSignalSpy>

#include <scopes.h>
#include <scope.h>
#include <categories.h>
#include <resultsmodel.h>
#include <previewmodel.h>
#include <previewstack.h>
#include <previewwidgetmodel.h>

using namespace scopes_ng;

void checkedFirstResult(Scope* scope, unity::scopes::Result::SPtr& result, bool& success)
{
    // ensure categories have > 0 rows
    auto categories = scope->categories();
    QVERIFY(categories->rowCount() > 0);
    QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
    QVERIFY(results_var.canConvert<ResultsModel*>());

    // ensure results have some data
    auto results = results_var.value<ResultsModel*>();
    QVERIFY(results->rowCount() > 0);
    auto result_var = results->data(results->index(0), ResultsModel::RoleResult);
    QCOMPARE(result_var.isNull(), false);
    result = result_var.value<std::shared_ptr<unity::scopes::Result>>();
    success = true;
}

bool getFirstResult(Scope* scope, unity::scopes::Result::SPtr& result)
{
    bool success = false;
    checkedFirstResult(scope, result, success);
    return success;
}

void performSearch(Scope* scope, QString const& searchString)
{
    QCOMPARE(scope->searchInProgress(), false);
    QSignalSpy spy(scope, SIGNAL(searchInProgressChanged()));
    // perform a search
    scope->setSearchQuery(searchString);
    QVERIFY(scope->searchInProgress() || spy.count() > 1);
    if (scope->searchInProgress()) {
        // wait for the search to finish
        QVERIFY(spy.wait());
    }
    QCOMPARE(scope->searchInProgress(), false);
}

void waitForResultsChange(Scope* scope)
{
    QCOMPARE(scope->searchInProgress(), false);
    // wait for the search to finish
    QSignalSpy spy(scope, SIGNAL(searchInProgressChanged()));
    QVERIFY(spy.wait());
    if(spy.size() == 1) {
        QVERIFY(spy.wait());
    }
    QCOMPARE(scope->searchInProgress(), false);
}

bool previewForFirstResult(Scope* scope, QString const& searchString, QScopedPointer<PreviewStack>& preview_stack)
{
    performSearch(scope, searchString);

    unity::scopes::Result::SPtr result;
    if (!getFirstResult(scope, result))
        return false;
    preview_stack.reset(static_cast<PreviewStack*>(scope->preview(QVariant::fromValue(result))));

    return true;
}
