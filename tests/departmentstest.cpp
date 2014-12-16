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

#include <scope-harness/registry/pre-existing-registry.h>
#include <scope-harness/test-utils.h>

namespace sc = unity::scopes;
namespace sh = unity::scopeharness;
namespace shr = unity::scopeharness::registry;
namespace ss = unity::shell::scopes;
namespace ng = scopes_ng;

class DepartmentsTest : public QObject
{
    Q_OBJECT
private:
    QScopedPointer<ng::Scopes> m_scopes;
    ng::Scope::Ptr m_scope;
    ng::Scope::Ptr m_scope_navs;
    ng::Scope::Ptr m_scope_flipflop;
    shr::Registry::UPtr m_registry;

private Q_SLOTS:
    void initTestCase()
    {
        qputenv("UNITY_SCOPES_NO_WAIT_LOCATION", "1");
        m_registry.reset(new shr::PreExistingRegistry(TEST_RUNTIME_CONFIG));
        m_registry->start();
    }

    void cleanupTestCase()
    {
        m_registry.reset();
    }

    void init()
    {
        QStringList favs;
        favs << "scope://mock-scope-departments" << "scope://mock-scope-double-nav" << "scope://mock-scope-departments-flipflop";
        sh::setFavouriteScopes(favs);

        m_scopes.reset(new ng::Scopes(nullptr));
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
        m_scope = m_scopes->getScopeById("mock-scope-departments");
        QVERIFY(m_scope != nullptr);
        m_scope->setActive(true);

        m_scope_navs = m_scopes->getScopeById("mock-scope-double-nav");
        QVERIFY(m_scope_navs != nullptr);
        m_scope_navs->setActive(true);

        m_scope_flipflop = m_scopes->getScopeById("mock-scope-departments-flipflop");
        QVERIFY(m_scope_flipflop != nullptr);
        m_scope_flipflop->setActive(true);

        QTRY_COMPARE(m_scope->searchInProgress(), false);
        QTRY_COMPARE(m_scope_navs->searchInProgress(), false);
        QTRY_COMPARE(m_scope_flipflop->searchInProgress(), false);
    }

    void cleanup()
    {
        m_scopes.reset();
        m_scope.reset();
        m_scope_navs.reset();
        m_scope_flipflop.reset();
    }

    void testNoDepartments()
    {
        sh::performSearch(m_scope, QString("foo"));

        QCOMPARE(m_scope->hasNavigation(), false);
        QCOMPARE(m_scope->hasAltNavigation(), false);
    }

    void testRootDepartment()
    {
        sh::performSearch(m_scope, QString(""));

        QCOMPARE(m_scope->hasNavigation(), true);
        QCOMPARE(m_scope->hasAltNavigation(), false);
        QCOMPARE(m_scope->currentNavigationId(), QString(""));
        QScopedPointer<ss::NavigationInterface> departmentModel(m_scope->getNavigation(m_scope->currentNavigationId()));
        QVERIFY(departmentModel != nullptr);

        QVERIFY(departmentModel->navigationId().isEmpty());
        QCOMPARE(departmentModel->label(), QString("All departments"));
        QCOMPARE(departmentModel->allLabel(), QString(""));
        QCOMPARE(departmentModel->parentNavigationId(), QString());
        QCOMPARE(departmentModel->parentLabel(), QString());
        QCOMPARE(departmentModel->loaded(), true);
        QCOMPARE(departmentModel->isRoot(), true);
        QCOMPARE(departmentModel->hidden(), false);

        QCOMPARE(departmentModel->rowCount(), 5);
        QModelIndex idx;

        idx = departmentModel->index(0);
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleNavigationId), QVariant(QString("books")));
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleLabel), QVariant(QString("Books")));
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleHasChildren), QVariant(true));
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleIsActive), QVariant(false));

        idx = departmentModel->index(4);
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleNavigationId), QVariant(QString("toys")));
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleLabel), QVariant(QString("Toys, Children & Baby")));
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleHasChildren), QVariant(true));
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleIsActive), QVariant(false));
    }

    void testChildDepartmentModel()
    {
        sh::performSearch(m_scope, QString(""));

        QCOMPARE(m_scope->currentNavigationId(), QString(""));
        QScopedPointer<ss::NavigationInterface> departmentModel(m_scope->getNavigation(QString("toys")));
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

        m_scope->setNavigationState(departmentModel->navigationId(), false);
        QVERIFY(spy.wait());

        QCOMPARE(departmentModel->rowCount(), 2);
        QCOMPARE(departmentModel->loaded(), true);
        QCOMPARE(departmentModel->isRoot(), false);
    }

    void testLeafActivationUpdatesModel()
    {
        sh::performSearch(m_scope, QString(""));

        QCOMPARE(m_scope->currentNavigationId(), QString(""));
        QSignalSpy spy(m_scope.data(), SIGNAL(searchInProgressChanged()));
        QScopedPointer<ss::NavigationInterface> navModel(m_scope->getNavigation(QString("books")));
        m_scope->setNavigationState(navModel->navigationId(), false);
        QVERIFY(spy.wait());
        QCOMPARE(m_scope->searchInProgress(), false);
        QScopedPointer<ss::NavigationInterface> departmentModel(m_scope->getNavigation(QString("books")));
        QCOMPARE(departmentModel->isRoot(), false);

        navModel.reset(m_scope->getNavigation(QString("books-audio")));
        // this is a leaf department, so activating it should update the parent model
        m_scope->setNavigationState(navModel->navigationId(), false);
        QVERIFY(spy.wait());
        QCOMPARE(m_scope->searchInProgress(), false);
        QCOMPARE(departmentModel->isRoot(), false);

        bool foundAudiobooks = false;
        for (int i = 0; i < departmentModel->rowCount(); i++) {
            QModelIndex idx(departmentModel->index(i));
            QVariant data = departmentModel->data(idx, ng::Department::Roles::RoleNavigationId);
            if (data.toString() == QString("books-audio")) {
                QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleIsActive).toBool(), true);
                foundAudiobooks = true;
            }
        }
        QCOMPARE(foundAudiobooks, true);
    }

    void testGoingBack()
    {
        sh::performSearch(m_scope, QString("x"));

        QCOMPARE(m_scope->currentNavigationId(), QString(""));
        QSignalSpy spy(m_scope.data(), SIGNAL(searchInProgressChanged()));
        QScopedPointer<ss::NavigationInterface> navModel(m_scope->getNavigation(QString("books")));
        m_scope->setNavigationState(navModel->navigationId(), false);
        QVERIFY(spy.wait());
        QCOMPARE(m_scope->searchInProgress(), false);
        QScopedPointer<ss::NavigationInterface> departmentModel(m_scope->getNavigation(QString("books")));
        QCOMPARE(departmentModel->isRoot(), false);

        // get the root again without actually loading the department
        departmentModel.reset(m_scope->getNavigation(departmentModel->parentNavigationId()));
        QCOMPARE(departmentModel->isRoot(), true);
        QEXPECT_FAIL("", "We have the department in cache, to it kind of is loaded", Continue);
        QCOMPARE(departmentModel->loaded(), false);
    }

    void testIncompleteTreeOnLeaf()
    {
        sh::performSearch(m_scope, QString(""));

        QScopedPointer<ss::NavigationInterface> navModel;
        QScopedPointer<ss::NavigationInterface> departmentModel;

        QCOMPARE(m_scope->currentNavigationId(), QString(""));
        QCOMPARE(m_scope->hasNavigation(), true);

        QSignalSpy spy(m_scope.data(), SIGNAL(searchInProgressChanged()));
        navModel.reset(m_scope->getNavigation(QString("toys")));
        m_scope->setNavigationState(navModel->navigationId(), false);
        QVERIFY(spy.wait());
        QCOMPARE(m_scope->searchInProgress(), false);

        departmentModel.reset(m_scope->getNavigation(QString("toys")));
        QCOMPARE(departmentModel->isRoot(), false);
        QCOMPARE(departmentModel->rowCount(), 2);

        navModel.reset(m_scope->getNavigation(QString("toys-games")));
        m_scope->setNavigationState(navModel->navigationId(), false);
        QVERIFY(spy.wait());
        QCOMPARE(m_scope->searchInProgress(), false);

        // after getting the parent department model, it should still have
        // all the leaves, even though the leaf served just itself
        departmentModel.reset(m_scope->getNavigation(QString("toys")));
        QCOMPARE(departmentModel->isRoot(), false);
        QCOMPARE(departmentModel->rowCount(), 2);
    }

    void testDoubleNavigation()
    {
        QCOMPARE(m_scope_navs->hasNavigation(), true);
        QCOMPARE(m_scope_navs->hasAltNavigation(), true);
        QCOMPARE(m_scope_navs->currentNavigationId(), QString(""));
        QCOMPARE(m_scope_navs->currentAltNavigationId(), QString("featured"));
        QScopedPointer<ss::NavigationInterface> departmentModel(m_scope_navs->getNavigation(m_scope_navs->currentNavigationId()));
        QVERIFY(departmentModel != nullptr);

        QVERIFY(!m_scope_navs->currentAltNavigationId().isEmpty());
        QScopedPointer<ss::NavigationInterface> sortOrderModel(m_scope_navs->getAltNavigation(""));
        QVERIFY(sortOrderModel != nullptr);

        QCOMPARE(sortOrderModel->navigationId(), QString(""));
        QCOMPARE(sortOrderModel->label(), QString("Sort Order"));
        QCOMPARE(sortOrderModel->allLabel(), QString(""));
        QCOMPARE(sortOrderModel->parentNavigationId(), QString());
        QCOMPARE(sortOrderModel->parentLabel(), QString());
        QCOMPARE(sortOrderModel->loaded(), true);
        QCOMPARE(sortOrderModel->isRoot(), true);
        QCOMPARE(sortOrderModel->hidden(), true);

        QCOMPARE(sortOrderModel->rowCount(), 3);
        QModelIndex idx;

        idx = sortOrderModel->index(0);
        QCOMPARE(sortOrderModel->data(idx, ng::Department::Roles::RoleNavigationId), QVariant(QString("featured")));
        QCOMPARE(sortOrderModel->data(idx, ng::Department::Roles::RoleLabel), QVariant(QString("Featured")));
        QCOMPARE(sortOrderModel->data(idx, ng::Department::Roles::RoleHasChildren), QVariant(false));
        QCOMPARE(sortOrderModel->data(idx, ng::Department::Roles::RoleIsActive), QVariant(true));

        idx = sortOrderModel->index(2);
        QCOMPARE(sortOrderModel->data(idx, ng::Department::Roles::RoleNavigationId), QVariant(QString("best")));
        QCOMPARE(sortOrderModel->data(idx, ng::Department::Roles::RoleLabel), QVariant(QString("Best sellers")));
        QCOMPARE(sortOrderModel->data(idx, ng::Department::Roles::RoleHasChildren), QVariant(false));
        QCOMPARE(sortOrderModel->data(idx, ng::Department::Roles::RoleIsActive), QVariant(false));
    }

    void testDoubleNavChangeActive()
    {
        QCOMPARE(m_scope_navs->currentAltNavigationId(), QString("featured"));
        QScopedPointer<ss::NavigationInterface> sortOrderModel(m_scope_navs->getAltNavigation(""));
        QVERIFY(sortOrderModel != nullptr);
        QCOMPARE(sortOrderModel->loaded(), true);
        QCOMPARE(sortOrderModel->rowCount(), 3);

        QModelIndex idx(sortOrderModel->index(1));
        QCOMPARE(sortOrderModel->data(idx, ng::Department::Roles::RoleNavigationId), QVariant(QString("top")));
        QCOMPARE(sortOrderModel->data(idx, ng::Department::Roles::RoleIsActive), QVariant(false));

        // perform a query for the other navigation
        QSignalSpy spy(m_scope_navs.data(), SIGNAL(searchInProgressChanged()));
        m_scope_navs->setNavigationState("top", true);
        QVERIFY(spy.wait());

        // the model should be updated
        QCOMPARE(sortOrderModel->data(idx, ng::Department::Roles::RoleNavigationId), QVariant(QString("top")));
        QCOMPARE(sortOrderModel->data(idx, ng::Department::Roles::RoleIsActive), QVariant(true));
    }

    void testDepartmentDissapear()
    {
        QCOMPARE(m_scope_flipflop->hasNavigation(), true);
        QCOMPARE(m_scope_flipflop->hasAltNavigation(), false);
        QCOMPARE(m_scope_flipflop->currentNavigationId(), QString(""));

        QScopedPointer<ss::NavigationInterface> departmentModel(m_scope_flipflop->getNavigation(m_scope_flipflop->currentNavigationId()));
        QVERIFY(departmentModel != nullptr);

        QVERIFY(departmentModel->navigationId().isEmpty());
        QCOMPARE(departmentModel->label(), QString("All departments"));
        QCOMPARE(departmentModel->allLabel(), QString(""));
        QCOMPARE(departmentModel->parentNavigationId(), QString());
        QCOMPARE(departmentModel->parentLabel(), QString());
        QCOMPARE(departmentModel->loaded(), true);
        QCOMPARE(departmentModel->isRoot(), true);
        QCOMPARE(departmentModel->hidden(), false);

        QCOMPARE(departmentModel->rowCount(), 5);

        sh::refreshSearch(m_scope_flipflop);

        // one department removed
        QCOMPARE(departmentModel->rowCount(), 4);

        QModelIndex idx;

        idx = departmentModel->index(0);
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleNavigationId), QVariant(QString("books")));
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleLabel), QVariant(QString("Books")));
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleHasChildren), QVariant(true));
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleIsActive), QVariant(false));

        idx = departmentModel->index(3);
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleNavigationId), QVariant(QString("toys")));
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleLabel), QVariant(QString("Toys, Children & Baby")));
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleHasChildren), QVariant(true));
        QCOMPARE(departmentModel->data(idx, ng::Department::Roles::RoleIsActive), QVariant(false));
    }

};

QTEST_GUILESS_MAIN(DepartmentsTest)
#include <departmentstest.moc>
