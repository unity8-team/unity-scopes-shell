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
using namespace unity::shell::scopes;

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
        QSignalSpy spy(m_scopes.data(), SIGNAL(loadedChanged()));
        // wait till the registry spawns
        QVERIFY(spy.wait());
        QCOMPARE(m_scopes->loaded(), true);
        // should have one scope now
        QVERIFY(m_scopes->rowCount() > 1);

        // get scope proxy
        m_scope = qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(QString("mock-scope-departments")));
        QVERIFY(m_scope != nullptr);
        m_scope->setActive(true);
    }

    void cleanup()
    {
        m_scopes.reset();
        m_scope = nullptr;
    }

    void testNoDepartments()
    {
        performSearch(m_scope, QString("foo"));

        QCOMPARE(m_scope->hasNavigation(), false);
    }

    void testRootDepartment()
    {
        performSearch(m_scope, QString(""));

        QCOMPARE(m_scope->hasNavigation(), true);
        QCOMPARE(m_scope->currentNavigationId(), QString(""));
        QScopedPointer<NavigationInterface> departmentModel(m_scope->getNavigation(m_scope->currentNavigationId()));
        QVERIFY(departmentModel != nullptr);

        QVERIFY(departmentModel->navigationId().isEmpty());
        QCOMPARE(departmentModel->label(), QString("All departments"));
        QCOMPARE(departmentModel->allLabel(), QString(""));
        QCOMPARE(departmentModel->parentNavigationId(), QString());
        QCOMPARE(departmentModel->parentLabel(), QString());
        QCOMPARE(departmentModel->loaded(), true);
        QCOMPARE(departmentModel->isRoot(), true);

        QCOMPARE(departmentModel->rowCount(), 5);
        QModelIndex idx;

        idx = departmentModel->index(0);
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleNavigationId), QVariant(QString("books")));
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleLabel), QVariant(QString("Books")));
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleHasChildren), QVariant(true));
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleIsActive), QVariant(false));

        idx = departmentModel->index(4);
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleNavigationId), QVariant(QString("toys")));
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleLabel), QVariant(QString("Toys, Children & Baby")));
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleHasChildren), QVariant(true));
        QCOMPARE(departmentModel->data(idx, Department::Roles::RoleIsActive), QVariant(false));
    }

    void testChildDepartmentModel()
    {
        performSearch(m_scope, QString(""));

        QCOMPARE(m_scope->currentNavigationId(), QString(""));
        QScopedPointer<NavigationInterface> departmentModel(m_scope->getNavigation(QString("toys")));
        QVERIFY(departmentModel != nullptr);

        QSignalSpy spy(departmentModel.data(), SIGNAL(loadedChanged()));

        QCOMPARE(departmentModel->navigationId(), QString("toys"));
        QCOMPARE(departmentModel->label(), QString("Toys, Children & Baby"));
        QCOMPARE(departmentModel->allLabel(), QString(""));
        QCOMPARE(departmentModel->parentNavigationId(), QString(""));
        QCOMPARE(departmentModel->parentLabel(), QString("All departments"));
        QCOMPARE(departmentModel->loaded(), false);
        QCOMPARE(departmentModel->isRoot(), false);

        QCOMPARE(departmentModel->rowCount(), 0);

        m_scope->performQuery(departmentModel->query());
        QVERIFY(spy.wait());

        QCOMPARE(departmentModel->rowCount(), 2);
        QCOMPARE(departmentModel->loaded(), true);
        QCOMPARE(departmentModel->isRoot(), false);
    }

    void testLeafActivationUpdatesModel()
    {
        performSearch(m_scope, QString(""));

        QCOMPARE(m_scope->currentNavigationId(), QString(""));
        QSignalSpy spy(m_scope, SIGNAL(searchInProgressChanged()));
        QScopedPointer<NavigationInterface> navModel(m_scope->getNavigation(QString("books")));
        m_scope->performQuery(navModel->query());
        QVERIFY(spy.wait());
        QCOMPARE(m_scope->searchInProgress(), false);
        QScopedPointer<NavigationInterface> departmentModel(m_scope->getNavigation(QString("books")));
        QCOMPARE(departmentModel->isRoot(), false);

        navModel.reset(m_scope->getNavigation(QString("books-audio")));
        // this is a leaf department, so activating it should update the parent model
        m_scope->performQuery(navModel->query());
        QVERIFY(spy.wait());
        QCOMPARE(m_scope->searchInProgress(), false);
        QCOMPARE(departmentModel->isRoot(), false);

        bool foundAudiobooks = false;
        for (int i = 0; i < departmentModel->rowCount(); i++) {
            QModelIndex idx(departmentModel->index(i));
            QVariant data = departmentModel->data(idx, Department::Roles::RoleNavigationId);
            if (data.toString() == QString("books-audio")) {
                QCOMPARE(departmentModel->data(idx, Department::Roles::RoleIsActive).toBool(), true);
                foundAudiobooks = true;
            }
        }
        QCOMPARE(foundAudiobooks, true);
    }

    void testGoingBack()
    {
        performSearch(m_scope, QString(""));

        QCOMPARE(m_scope->currentNavigationId(), QString(""));
        QSignalSpy spy(m_scope, SIGNAL(searchInProgressChanged()));
        QScopedPointer<NavigationInterface> navModel(m_scope->getNavigation(QString("books")));
        m_scope->performQuery(navModel->query());
        QVERIFY(spy.wait());
        QCOMPARE(m_scope->searchInProgress(), false);
        QScopedPointer<NavigationInterface> departmentModel(m_scope->getNavigation(QString("books")));
        QCOMPARE(departmentModel->isRoot(), false);

        // get the root again without actually loading the department
        departmentModel.reset(m_scope->getNavigation(departmentModel->parentNavigationId()));
        QCOMPARE(departmentModel->isRoot(), true);
        QEXPECT_FAIL("", "We have the department in cache, to it kind of is loaded", Continue);
        QCOMPARE(departmentModel->loaded(), false);
    }

    void testIncompleteTreeOnLeaf()
    {
        QScopedPointer<NavigationInterface> navModel;
        QScopedPointer<NavigationInterface> departmentModel;
        performSearch(m_scope, QString(""));

        QCOMPARE(m_scope->currentNavigationId(), QString(""));
        QCOMPARE(m_scope->hasNavigation(), true);

        QSignalSpy spy(m_scope, SIGNAL(searchInProgressChanged()));
        navModel.reset(m_scope->getNavigation(QString("toys")));
        m_scope->performQuery(navModel->query());
        QVERIFY(spy.wait());
        QCOMPARE(m_scope->searchInProgress(), false);

        departmentModel.reset(m_scope->getNavigation(QString("toys")));
        QCOMPARE(departmentModel->isRoot(), false);
        QCOMPARE(departmentModel->rowCount(), 2);

        navModel.reset(m_scope->getNavigation(QString("toys-games")));
        m_scope->performQuery(navModel->query());
        QVERIFY(spy.wait());
        QCOMPARE(m_scope->searchInProgress(), false);

        // after getting the parent department model, it should still have
        // all the leaves, even though the leaf served just itself
        departmentModel.reset(m_scope->getNavigation(QString("toys")));
        QCOMPARE(departmentModel->isRoot(), false);
        QCOMPARE(departmentModel->rowCount(), 2);
    }

};

QTEST_GUILESS_MAIN(DepartmentsTest)
#include <departmentstest.moc>
