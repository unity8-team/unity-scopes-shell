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
#include <QJsonValue>
#include <QJsonObject>
#include <QThread>
#include <QScopedPointer>
#include <QSignalSpy>
#include <QVariantList>
#include <QDBusConnection>

#include <scopes.h>
#include <scope.h>
#include <categories.h>
#include <resultsmodel.h>
#include <previewmodel.h>
#include <previewstack.h>
#include <previewwidgetmodel.h>
#include <department.h>

#include "registry-spawner.h"
#include "test-utils.h"

using namespace scopes_ng;

class DepartmentsTest : public QObject
{
    Q_OBJECT
private:
    QScopedPointer<Scopes> m_scopes;
    Scope* m_scope;
    QScopedPointer<RegistrySpawner> m_registry;

private Q_SLOTS:
    void initTestCase()
    {
        m_registry.reset(new RegistrySpawner);
    }

    void cleanupTestCase()
    {
        m_registry.reset();
    }

    void init()
    {
        m_scopes.reset(new Scopes(nullptr));
        // no scopes on startup
        QCOMPARE(m_scopes->rowCount(), 0);
        QCOMPARE(m_scopes->loaded(), false);
        QSignalSpy spy(m_scopes.data(), SIGNAL(loadedChanged(bool)));
        // wait till the registry spawns
        QVERIFY(spy.wait());
        QCOMPARE(m_scopes->loaded(), true);
        // should have one scope now
        QVERIFY(m_scopes->rowCount() > 1);

        // get scope proxy
        QVariant scope_var = m_scopes->data(m_scopes->index(0), Scopes::Roles::RoleScope);
        QVERIFY(scope_var.canConvert<Scope*>());
        m_scope = scope_var.value<Scope*>();
    }

    void cleanup()
    {
        m_scopes.reset();
        m_scope = nullptr;
    }

    void testScopeCommunication()
    {
        performSearch(m_scope, QString(""));

        // ensure categories have > 0 rows
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());
        QCOMPARE(categories->data(categories->index(0), Categories::Roles::RoleName), QVariant(QString("Category 1")));
        QCOMPARE(categories->data(categories->index(0), Categories::Roles::RoleIcon), QVariant(QString("")));

        // ensure results have some data
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);
    }

    void testScopesGet()
    {
        unity::shell::scopes::ScopeInterface* scope = m_scopes->getScope(0);
        QVERIFY(scope);

        // try incorrect index as well
        scope = m_scopes->getScope(65536);
        QVERIFY(!scope);
        scope = m_scopes->getScope(-1);
        QVERIFY(!scope);

        // try to get by scope id
        scope = m_scopes->getScope(QString("mock-scope"));
        QVERIFY(scope);

        scope = m_scopes->getScope(QString("non-existing"));
        QVERIFY(!scope);
    }

};

QTEST_GUILESS_MAIN(DepartmentsTest)
#include <departmentstest.moc>
