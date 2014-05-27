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

    void testNoDepartments()
    {
        performSearch(m_scope, QString(""));

        QCOMPARE(m_scope->hasDepartments(), false);
        QVERIFY(m_scope->getDepartment(QString()) == nullptr);
    }

    void testRootDepartment()
    {
        performSearch(m_scope, QString("dep-query"));

        QCOMPARE(m_scope->hasDepartments(), true);
        QCOMPARE(m_scope->currentDepartment(), QString(""));
        QScopedPointer<Department> departmentModel(m_scope->getDepartment(m_scope->currentDepartment()));
        QVERIFY(departmentModel != nullptr);

        QVERIFY(departmentModel->departmentId().isEmpty());
        QCOMPARE(departmentModel->label(), QString("All departments"));
        QCOMPARE(departmentModel->allLabel(), QString(""));
        QCOMPARE(departmentModel->parentId(), QString());
        QCOMPARE(departmentModel->parentLabel(), QString());
        QCOMPARE(departmentModel->loaded(), true);

        QCOMPARE(departmentModel->rowCount(), 5);
        QModelIndex idx;

        idx = departmentModel->index(0);
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleDepartmentId), QVariant(QString("books")));
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleLabel), QVariant(QString("Books")));
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleHasChildren), QVariant(true));
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleIsActive), QVariant(false));

        idx = departmentModel->index(4);
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleDepartmentId), QVariant(QString("toys")));
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleLabel), QVariant(QString("Toys, Children & Baby")));
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleHasChildren), QVariant(true));
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleIsActive), QVariant(false));
    }

    void testChildDepartmentModel()
    {
        performSearch(m_scope, QString("dep-query"));

        QCOMPARE(m_scope->currentDepartment(), QString(""));
        QScopedPointer<Department> departmentModel(m_scope->getDepartment(QString("toys")));
        QVERIFY(departmentModel != nullptr);

        QSignalSpy spy(departmentModel.data(), SIGNAL(loadedChanged()));

        QCOMPARE(departmentModel->departmentId(), QString("toys"));
        QCOMPARE(departmentModel->label(), QString("Toys, Children & Baby"));
        QCOMPARE(departmentModel->allLabel(), QString(""));
        QCOMPARE(departmentModel->parentId(), QString(""));
        QCOMPARE(departmentModel->parentLabel(), QString("All departments"));
        QCOMPARE(departmentModel->loaded(), false);

        QCOMPARE(departmentModel->rowCount(), 0);

        m_scope->loadDepartment(QString("toys"));
        QVERIFY(spy.wait());
    }

};

QTEST_GUILESS_MAIN(DepartmentsTest)
#include <departmentstest.moc>
