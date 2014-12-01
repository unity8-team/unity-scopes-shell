/*
 * Copyright (C) 2014 Canonical, Ltd.
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
 *  Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <QObject>
#include <QTest>
#include <QScopedPointer>
#include <QSignalSpy>

#include <categories.h>
#include <scopes.h>
#include <scope.h>
#include <overviewresults.h>
#include <unity/shell/scopes/ScopeInterface.h>

#include <scope-harness/pre-existing-registry.h>
#include <scope-harness/test-utils.h>

namespace ng = scopes_ng;
namespace sh = unity::scopeharness;
using namespace unity::shell::scopes;

class FavoritesTest: public QObject
{
    Q_OBJECT
private:
    QScopedPointer<ng::Scopes> m_scopes;
    ng::Scope* m_overviewScope;
    sh::Registry::UPtr m_registry;

private Q_SLOTS:
    void initTestCase()
    {
        m_registry.reset(new sh::PreExistingRegistry(TEST_RUNTIME_CONFIG));
        m_registry->start();
    }

    void cleanupTestCase()
    {
        m_registry.reset();
    }

    void init()
    {
        sh::setFavouriteScopes(QStringList());

        m_scopes.reset(new ng::Scopes(nullptr));

        // no scopes on startup
        QCOMPARE(m_scopes->rowCount(), 0);
        QCOMPARE(m_scopes->loaded(), false);
        QSignalSpy spy(m_scopes.data(), SIGNAL(loadedChanged()));

        // wait till the registry spawns
        QVERIFY(spy.wait());
        QCOMPARE(m_scopes->loaded(), true);

        // no scopes after startup, since favorites empty
        QCOMPARE(m_scopes->rowCount(), 0);

        // get overview scope proxy
        m_overviewScope = qobject_cast<scopes_ng::Scope*>(m_scopes->overviewScope());
        QVERIFY(m_overviewScope!= nullptr);
        m_overviewScope->setActive(true);
    }

    void testFavoritePropertyUpdates()
    {
        QStringList favs;
        favs << "scope://mock-scope-departments";
        sh::setFavouriteScopes(favs);

        // should have one scope now
        QTRY_COMPARE(m_scopes->rowCount(), 1);
        auto scope1 = qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(QString("mock-scope-departments")));
        QVERIFY(scope1 != nullptr);
        QCOMPARE(scope1->favorite(), true);

        favs << "scope://mock-scope-double-nav";
        sh::setFavouriteScopes(favs);

        // should have two scopes now
        QTRY_COMPARE(m_scopes->rowCount(), 2);
        auto scope2 = qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(QString("mock-scope-double-nav")));
        QVERIFY(scope2 != nullptr);
        QCOMPARE(scope2->favorite(), true);

        // unfavorite 1st scope
        QSignalSpy spy(scope1, SIGNAL(favoriteChanged(bool)));
        QSignalSpy spy2(scope1, SIGNAL(destroyed(QObject *)));
        scope1->setFavorite(false);
        QTRY_COMPARE(spy.count(), 1);
        QTRY_COMPARE(m_scopes->rowCount(), 1);
        QVERIFY(m_scopes->getScopeById("mock-scope-departments") == nullptr);
        QCOMPARE(m_scopes->data(m_scopes->index(0), ng::Scopes::RoleId), QVariant(QString("mock-scope-double-nav")));

        // the scope should be destroyed after un-favoriting
        QTRY_COMPARE(spy2.count(), 1);

        // favorite a scope
        auto overviewScope = m_scopes->overviewScope();
        QVERIFY(overviewScope != nullptr);
        connect(overviewScope, &unity::shell::scopes::ScopeInterface::openScope, [](unity::shell::scopes::ScopeInterface* scope) {
                QCOMPARE(scope->favorite(), false);
                scope->setFavorite(true);
        });

        QVERIFY(m_scopes->getScopeById("mock-scope") == nullptr);
        overviewScope->performQuery("scope://mock-scope");
        QTRY_VERIFY(m_scopes->getScopeById("mock-scope") != nullptr);
    }

    void testFavoritesOverviewUpdates()
    {
        auto categories = m_overviewScope->categories();
        QVERIFY(categories->rowCount() > 0);
        QCOMPARE(categories->data(categories->index(0), ng::Categories::Roles::RoleCategoryId), QVariant(QString("favorites")));

        QVariant results_var = categories->data(categories->index(0), ng::Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ng::OverviewResultsModel*>());
        ng::OverviewResultsModel* results = results_var.value<ng::OverviewResultsModel*>();
        QCOMPARE(results->rowCount(), 0);

        // favorite one scope, check if it appears in the favorites model
        QStringList favs;
        favs << "scope://mock-scope-departments";
        sh::setFavouriteScopes(favs);

        QTRY_COMPARE(results->rowCount(), 1);

        // unfavorite it, verify it disappears from favorites model
        sh::setFavouriteScopes(QStringList());
        QTRY_COMPARE(results->rowCount(), 0);
    }

    void testFavoritesReordering()
    {
        QStringList favs;
        favs << "scope://mock-scope-departments" << "scope://mock-scope-double-nav" << "scope://mock-scope";
        setFavouriteScopes(favs);

        // should have one scope now
        QTRY_COMPARE(m_scopes->rowCount(), 3);
        QTRY_COMPARE(qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(0))->id(), QString("mock-scope-departments"));
        QTRY_COMPARE(qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(1))->id(), QString("mock-scope-double-nav"));
        QTRY_COMPARE(qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(2))->id(), QString("mock-scope"));

        {
            QSignalSpy spy(m_scopes.data(), SIGNAL(rowsMoved(const QModelIndex&, int, int, const QModelIndex&, int)));
            m_scopes->moveFavoriteTo("mock-scope", 1);

            // check new positions
            QTRY_COMPARE(spy.count(), 1);

            QTRY_COMPARE(qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(0))->id(), QString("mock-scope-departments"));
            QTRY_COMPARE(qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(1))->id(), QString("mock-scope"));
            QTRY_COMPARE(qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(2))->id(), QString("mock-scope-double-nav"));

            // check overview model
            auto categories = m_overviewScope->categories();
            QVERIFY(categories->rowCount() > 0);
            QCOMPARE(categories->data(categories->index(0), Categories::Roles::RoleCategoryId), QVariant(QString("favorites")));

            QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
            QVERIFY(results_var.canConvert<OverviewResultsModel*>());
            OverviewResultsModel* results = results_var.value<OverviewResultsModel*>();
            QTRY_COMPARE(results->rowCount(), 3);

            QTRY_COMPARE(results->data(results->index(0), OverviewResultsModel::RoleScopeId), QVariant(QString("mock-scope-departments")));
            QTRY_COMPARE(results->data(results->index(1), OverviewResultsModel::RoleScopeId), QVariant(QString("mock-scope")));
            QTRY_COMPARE(results->data(results->index(2), OverviewResultsModel::RoleScopeId), QVariant(QString("mock-scope-double-nav")));
        }
        {
            QSignalSpy spy(m_scopes.data(), SIGNAL(rowsMoved(const QModelIndex&, int, int, const QModelIndex&, int)));
            m_scopes->moveFavoriteTo("mock-scope", 2);

            // check new positions
            QTRY_COMPARE(spy.count(), 1);

            QTRY_COMPARE(qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(0))->id(), QString("mock-scope-departments"));
            QTRY_COMPARE(qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(1))->id(), QString("mock-scope-double-nav"));
            QTRY_COMPARE(qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(2))->id(), QString("mock-scope"));

            // check overview model
            auto categories = m_overviewScope->categories();
            QVERIFY(categories->rowCount() > 0);
            QCOMPARE(categories->data(categories->index(0), Categories::Roles::RoleCategoryId), QVariant(QString("favorites")));

            QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
            QVERIFY(results_var.canConvert<OverviewResultsModel*>());
            OverviewResultsModel* results = results_var.value<OverviewResultsModel*>();
            QTRY_COMPARE(results->rowCount(), 3);

            QTRY_COMPARE(results->data(results->index(0), OverviewResultsModel::RoleScopeId), QVariant(QString("mock-scope-departments")));
            QTRY_COMPARE(results->data(results->index(1), OverviewResultsModel::RoleScopeId), QVariant(QString("mock-scope-double-nav")));
            QTRY_COMPARE(results->data(results->index(2), OverviewResultsModel::RoleScopeId), QVariant(QString("mock-scope")));
        }
    }

    void testGSettingsUpdates()
    {
        QStringList favs;
        favs << "scope://mock-scope-departments" << "scope://mock-scope-double-nav";
        sh::setFavouriteScopes(favs);

        // should have two scopes
        QTRY_COMPARE(m_scopes->rowCount(), 2);
        auto scope1 = qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(QString("mock-scope-departments")));
        QVERIFY(scope1 != nullptr);

        // un-favorite one scope
        scope1->setFavorite(false);
        QTRY_COMPARE(sh::getFavoriteScopes().size(), 1);
        QCOMPARE(sh::getFavoriteScopes().at(0), QString("scope://mock-scope-double-nav"));
    }

    void cleanup()
    {
        m_scopes.reset();
    }
};

QTEST_GUILESS_MAIN(FavoritesTest)
#include <favoritestest.moc>
