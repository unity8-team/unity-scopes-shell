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

#include <scopes.h>
#include <scope.h>
#include <overviewresults.h>
#include <unity/shell/scopes/ScopeInterface.h>

#include "registry-spawner.h"
#include "test-utils.h"

using namespace scopes_ng;
using namespace unity::shell::scopes;

class FavoritesTest: public QObject
{
    Q_OBJECT
private:
    QScopedPointer<Scopes> m_scopes;
    Scope* m_overviewScope;
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
        setFavouriteScopes(QStringList());

        m_scopes.reset(new Scopes(nullptr));

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
        setFavouriteScopes(favs);

        // should have one scope now
        QTRY_COMPARE(m_scopes->rowCount(), 1);
        auto scope1 = qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(QString("mock-scope-departments")));
        QVERIFY(scope1 != nullptr);
        QCOMPARE(scope1->favorite(), true);

        favs << "scope://mock-scope-double-nav";
        setFavouriteScopes(favs);

        // should have two scopes now
        QTRY_COMPARE(m_scopes->rowCount(), 2);
        auto scope2 = qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(QString("mock-scope-double-nav")));
        QVERIFY(scope2 != nullptr);
        QCOMPARE(scope2->favorite(), true);

        // unfavorite 1st scope
        QSignalSpy spy(scope1, SIGNAL(favoriteChanged(bool)));
        scope1->setFavorite(false);
        QTRY_COMPARE(spy.count(), 1);
        QTRY_COMPARE(m_scopes->rowCount(), 1);
        QVERIFY(m_scopes->getScopeById("mock-scope-departments") == nullptr);
        QCOMPARE(m_scopes->data(m_scopes->index(0), Scopes::RoleId), QVariant(QString("mock-scope-double-nav")));

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
        QCOMPARE(categories->data(categories->index(0), Categories::Roles::RoleCategoryId), QVariant(QString("favorites")));

        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<OverviewResultsModel*>());
        OverviewResultsModel* results = results_var.value<OverviewResultsModel*>();
        QCOMPARE(results->rowCount(), 0);

        // favorite one scope, check if it appears in the favorites model
        QStringList favs;
        favs << "scope://mock-scope-departments";
        setFavouriteScopes(favs);

        QTRY_COMPARE(results->rowCount(), 1);

        // unfavorite it, verify it disappears from favorites model
        setFavouriteScopes(QStringList());
        QTRY_COMPARE(results->rowCount(), 0);
    }

    void testGSettingsUpdates()
    {
        QStringList favs;
        favs << "scope://mock-scope-departments" << "scope://mock-scope-double-nav";
        setFavouriteScopes(favs);

        // should have two scopes
        QTRY_COMPARE(m_scopes->rowCount(), 2);
        auto scope1 = qobject_cast<scopes_ng::Scope*>(m_scopes->getScope(QString("mock-scope-departments")));
        QVERIFY(scope1 != nullptr);

        // un-facorite one scope
        scope1->setFavorite(false);
        QTRY_COMPARE(getFavoriteScopes().size(), 1);
        QCOMPARE(getFavoriteScopes().at(0), QString("scope://mock-scope-double-nav"));
    }

    void cleanup()
    {
        m_scopes.reset();
    }
};

QTEST_GUILESS_MAIN(FavoritesTest)
#include <favoritestest.moc>
