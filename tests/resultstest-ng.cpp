/*
 * Copyright (C) 2013 Canonical, Ltd.
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
#include <QProcess>
#include <QThread>
#include <QScopedPointer>

#include <scopes-ng/scopes.h>
#include <scopes-ng/scope.h>
#include <scopes-ng/categories.h>
#include <scopes-ng/resultsmodel.h>

using namespace scopes_ng;

class ResultsTestNg : public QObject
{
    Q_OBJECT
private:
    QScopedPointer<Scopes> m_scopes;
    Scope* m_scope;
    QProcess* m_registry;

private Q_SLOTS:
    void initTestCase()
    {
        // startup our private scope registry
        QString registryBin(TEST_SCOPEREGISTRY_BIN);
        QStringList arguments;
        arguments << TEST_RUNTIME_CONFIG;

        m_registry = new QProcess(this);
        m_registry->start(registryBin, arguments);
    }

    void cleanupTestCase()
    {
        m_registry->terminate();
        if (!m_registry->waitForFinished()) {
            m_registry->kill();
        }
    }

    void init()
    {
        m_scopes.reset(new Scopes(nullptr));
        // no scopes on startup
        QCOMPARE(m_scopes->rowCount(), 0);
        // wait till the registry spawns
        QTRY_COMPARE(m_scopes->loaded(), true);
        // should have one scope now
        QCOMPARE(m_scopes->rowCount(), 1);

        QVariant scope_var = m_scopes->data(m_scopes->index(0), Scopes::Roles::RoleScope);
        QVERIFY(scope_var.canConvert<Scope*>());

        // get scope proxy
        m_scope = scope_var.value<Scope*>();
    }

    void cleanup()
    {
        m_scopes.reset();
        m_scope = nullptr;
    }

    void testScopeCommunication()
    {
        QCOMPARE(m_scope->searchInProgress(), false);
        // perform a search
        m_scope->setSearchQuery(QString(""));
        QCOMPARE(m_scope->searchInProgress(), true);
        QTRY_COMPARE(m_scope->searchInProgress(), false);

        // ensure categories have > 0 rows
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());

        // ensure results have some data
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);
    }

    void testTwoSearches()
    {
        QCOMPARE(m_scope->searchInProgress(), false);
        // perform a search
        m_scope->setSearchQuery(QString(""));
        QCOMPARE(m_scope->searchInProgress(), true);
        QTRY_COMPARE(m_scope->searchInProgress(), false);

        // ensure categories have > 0 rows
        auto categories = m_scope->categories();
        auto categories_count = categories->rowCount();
        QVERIFY(categories_count > 0);

        m_scope->setSearchQuery(QString("foo"));
        QCOMPARE(m_scope->searchInProgress(), true);
        // wait for the search to finish
        QTRY_COMPARE(m_scope->searchInProgress(), false);

        // shouldn't create more nor fewer categories
        QVERIFY(categories->rowCount() == categories_count);
    }

    void testBasicResultData()
    {
        QCOMPARE(m_scope->searchInProgress(), false);
        // perform a search
        m_scope->setSearchQuery(QString(""));
        QCOMPARE(m_scope->searchInProgress(), true);
        QTRY_COMPARE(m_scope->searchInProgress(), false);

        // get ResultsModel instance
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleUri).toString(), QString("test:uri"));
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleDndUri).toString(), QString("test:dnd_uri"));
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleTitle).toString(), QString("result for: \"\""));
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleArt).toString(), QString("art"));
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleCategoryId), categories->data(categories->index(0), Categories::Roles::RoleCategoryId));
    }

    void testMetadataData()
    {
        QCOMPARE(m_scope->searchInProgress(), false);
        // perform a search
        m_scope->setSearchQuery(QString("metadata"));
        QCOMPARE(m_scope->searchInProgress(), true);
        QTRY_COMPARE(m_scope->searchInProgress(), false);

        // get ResultsModel instance
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleTitle).toString(), QString("result for: \"metadata\""));
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleSubtitle).toString(), QString("subtitle"));
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleEmblem).toString(), QString("emblem"));
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleAltRating).toString(), QString());
    }
};

QTEST_MAIN(ResultsTestNg)
#include <resultstest-ng.moc>
