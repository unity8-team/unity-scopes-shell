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

#include <Unity/resultsmodel.h>

#include <unity/shell/scopes/CategoriesInterface.h>
#include <unity/shell/scopes/ResultsModelInterface.h>

#include <scope-harness/scope-harness.h>
#include <scope-harness/test-utils.h>

namespace sh = unity::scopeharness;
namespace sc = unity::scopes;
namespace ss = unity::shell::scopes;

class CountObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit CountObject(QObject* parent = nullptr) : QObject(parent), m_count(0), m_waitingCount(0)
    {
        connect(&m_timer, &QTimer::timeout, this, &CountObject::asyncTimeout);
    }

Q_SIGNALS:
    void countChanged();

private Q_SLOTS:
    void asyncTimeout() { setCount(m_waitingCount); }

public:
    int count() const { return m_count; }

    void setCount(int newCount)
    {
        if (newCount != m_count) {
            m_count = newCount;
            Q_EMIT countChanged();
        }
    }

    void setCountAsync(int newCount)
    {
        m_waitingCount = newCount;
        m_timer.setSingleShot(true);
        m_timer.start(1);
    }

private:
    int m_count;
    int m_waitingCount;
    QTimer m_timer;
};

class ResultsTest : public QObject
{
    Q_OBJECT
private:
    sh::ScopeHarness::UPtr m_harness;


private Q_SLOTS:
    void initTestCase()
    {
        qputenv("UNITY_SCOPES_NO_WAIT_LOCATION", "1");
        m_harness = sh::ScopeHarness::newFromPreExistingConfig(TEST_RUNTIME_CONFIG);
    }

    void cleanupTestCase()
    {
        m_harness.reset();
    }

    void init()
    {
    }

    void cleanup()
    {
    }

    void testScopeCommunication()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("");

        // ensure categories have > 0 rows
        auto categories = resultsView->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ss::ResultsModelInterface*>());
        QCOMPARE(categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleName), QVariant(QString("Category 1")));
        QCOMPARE(categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleIcon), QVariant(QString("")));
        QVERIFY(categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleHeaderLink).toString().isNull());

        // ensure results have some data
        auto results = results_var.value<ss::ResultsModelInterface*>();
        QVERIFY(results->rowCount() > 0);
    }

//    void testScopesGet()
//    {
//        ss::ScopeInterface* scope = m_scopes->getScope(0);
//        QVERIFY(scope);
//
//        // try incorrect index as well
//        scope = m_scopes->getScope(65536);
//        QVERIFY(!scope);
//        scope = m_scopes->getScope(-1);
//        QVERIFY(!scope);
//
//        // try to get by scope id
//        scope = m_scopes->getScope(QString("mock-scope"));
//        QVERIFY(scope);
//
//        scope = m_scopes->getScope(QString("non-existing"));
//        QVERIFY(!scope);
//    }

    void testScopeProperties()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");

        QVERIFY(resultsView->scopeId() == "mock-scope");
        QVERIFY(resultsView->displayName() == "mock.DisplayName");
        QVERIFY(resultsView->iconHint() == "/mock.Icon");
        QVERIFY(resultsView->description() == "mock.Description");
        QVERIFY(resultsView->searchHint() == "mock.SearchHint");
        QVERIFY(resultsView->shortcut() =="mock.HotKey");
        QVERIFY(resultsView->searchQuery() == "");

        QVariantMap customizations(resultsView->customizations());
        QVERIFY(customizations.size() > 0);
        QCOMPARE(static_cast<QMetaType::Type>(customizations["page-header"].type()), QMetaType::QVariantMap);
        QVariantMap headerCustomizations(customizations["page-header"].toMap());
        QCOMPARE(headerCustomizations["logo"], QVariant("http://assets.ubuntu.com/sites/ubuntu/1110/u/img/logos/logo-ubuntu-orange.svg"));
        QCOMPARE(headerCustomizations["foreground-color"], QVariant("white"));
        QCOMPARE(headerCustomizations["background"], QVariant("color://black"));
        QCOMPARE(customizations["shape-images"], QVariant(false));

        resultsView->setActiveScope("mock-scope-ttl");

        QVERIFY(resultsView->scopeId() == "mock-scope-ttl");
        QVERIFY(resultsView->displayName() == "mock-ttl.DisplayName");
        QVERIFY(resultsView->iconHint() == "/mock-ttl.Icon");
        QVERIFY(resultsView->description() == "mock-ttl.Description");
        QVERIFY(resultsView->searchHint() == "");
        QVERIFY(resultsView->shortcut() == "");
        QVERIFY(resultsView->searchQuery() == "");
    }

    void testCategoryQuery()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("expansion-query");

        // ensure categories have > 0 rows
        auto categories = resultsView->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ss::ResultsModelInterface*>());
        QCOMPARE(categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleName), QVariant(QString("Category 1")));
        QCOMPARE(categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleIcon), QVariant(QString("")));
        QVERIFY(categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleHeaderLink).toString().startsWith("scope://"));
    }

    void testTwoSearches()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("");

        // ensure categories have > 0 rows
        auto categories = resultsView->categories();
        auto categories_count = categories->rowCount();
        QVERIFY(categories_count > 0);

        resultsView->setQuery("foo");

        // shouldn't create more nor fewer categories
        QVERIFY(categories->rowCount() == categories_count);
    }

    void testBasicResultData()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("");

        // get ResultsModel instance
        auto categories = resultsView->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ss::ResultsModelInterface*>());
        auto results = results_var.value<ss::ResultsModelInterface*>();
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleUri).toString(), QString("test:uri"));
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleDndUri).toString(), QString("test:dnd_uri"));
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleTitle).toString(), QString("result for: \"\""));
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleArt).toString(), QString("art"));
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleCategoryId), categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleCategoryId));
    }

    void testSessionId()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("");

        std::string lastSessionId;

        QVERIFY(!resultsView->sessionId().empty());
        QCOMPARE(resultsView->queryId(), 0);

        {
            auto categories = resultsView->categories();
            QVERIFY(categories->rowCount() > 0);
            QVariant results_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleResults);
            auto results = results_var.value<ss::ResultsModelInterface*>();
            QVERIFY(results->rowCount() > 0);

            auto idx = results->index(0);
            auto result = results->data(idx, ss::ResultsModelInterface::Roles::RoleResult).value<std::shared_ptr<sc::Result>>();

            auto sessionId = (*result)["session-id"].get_string();
            auto queryId = (*result)["query-id"].get_int();

            // mock scope should send session-id and query-id it received back via custom result's values
            QCOMPARE(sessionId, resultsView->sessionId());
            QCOMPARE(queryId, resultsView->queryId());
            QCOMPARE(queryId, 0);

            lastSessionId = sessionId;
        }

        // new search
        resultsView->setQuery("m");
        {
            auto categories = resultsView->categories();
            QVERIFY(categories->rowCount() > 0);
            QVariant results_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleResults);
            auto results = results_var.value<ss::ResultsModelInterface*>();
            QVERIFY(results->rowCount() > 0);

            auto idx = results->index(0);
            auto result = results->data(idx, ss::ResultsModelInterface::Roles::RoleResult).value<std::shared_ptr<sc::Result>>();

            auto sessionId = (*result)["session-id"].get_string();
            auto queryId = (*result)["query-id"].get_int();

            // mock scope should send session-id and query-id it received back via custom result's values
            QCOMPARE(sessionId, resultsView->sessionId());
            QCOMPARE(queryId, resultsView->queryId());
            QCOMPARE(queryId, 0);

            // new session id
            QVERIFY(sessionId != lastSessionId);

            lastSessionId = sessionId;
        }

        // appends to previous search
        resultsView->setQuery("met");
        {
            auto categories = resultsView->categories();
            QVERIFY(categories->rowCount() > 0);
            QVariant results_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleResults);
            auto results = results_var.value<ss::ResultsModelInterface*>();
            QVERIFY(results->rowCount() > 0);

            auto idx = results->index(0);
            auto result = results->data(idx, ss::ResultsModelInterface::Roles::RoleResult).value<std::shared_ptr<sc::Result>>();

            auto sessionId = (*result)["session-id"].get_string();
            auto queryId = (*result)["query-id"].get_int();

            // mock scope should send session-id and query-id it received back via custom result's values
            QCOMPARE(sessionId, resultsView->sessionId());
            QCOMPARE(queryId, resultsView->queryId());
            QCOMPARE(queryId, 1);

            // session id unchanged
            QVERIFY(sessionId == lastSessionId);

            lastSessionId = sessionId;
        }

        // removes characters from previous search
        resultsView->setQuery("m");
        {
            auto categories = resultsView->categories();
            QVERIFY(categories->rowCount() > 0);
            QVariant results_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleResults);
            auto results = results_var.value<ss::ResultsModelInterface*>();
            QVERIFY(results->rowCount() > 0);

            auto idx = results->index(0);
            auto result = results->data(idx, ss::ResultsModelInterface::Roles::RoleResult).value<std::shared_ptr<sc::Result>>();

            auto sessionId = (*result)["session-id"].get_string();
            auto queryId = (*result)["query-id"].get_int();

            // mock scope should send session-id and query-id it received back via custom result's values
            QCOMPARE(sessionId, resultsView->sessionId());
            QCOMPARE(queryId, resultsView->queryId());
            QCOMPARE(queryId, 2);

            // session id unchanged
            QVERIFY(sessionId == lastSessionId);

            lastSessionId = sessionId;
        }

        // new non-empty search again
        resultsView->setQuery("iron");
        {
            auto categories = resultsView->categories();
            QVERIFY(categories->rowCount() > 0);
            QVariant results_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleResults);
            auto results = results_var.value<ss::ResultsModelInterface*>();
            QVERIFY(results->rowCount() > 0);

            auto idx = results->index(0);
            auto result = results->data(idx, ss::ResultsModelInterface::Roles::RoleResult).value<std::shared_ptr<sc::Result>>();

            auto sessionId = (*result)["session-id"].get_string();
            auto queryId = (*result)["query-id"].get_int();

            // mock scope should send session-id and query-id it received back via custom result's values
            QCOMPARE(sessionId, resultsView->sessionId());
            QCOMPARE(queryId, resultsView->queryId());
            QCOMPARE(queryId, 0);

            // new session id
            QVERIFY(sessionId != lastSessionId);

            lastSessionId = sessionId;
        }

        // new empty search again
        resultsView->setQuery("");
        {
            auto categories = resultsView->categories();
            QVERIFY(categories->rowCount() > 0);
            QVariant results_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleResults);
            auto results = results_var.value<ss::ResultsModelInterface*>();
            QVERIFY(results->rowCount() > 0);

            auto idx = results->index(0);
            auto result = results->data(idx, ss::ResultsModelInterface::Roles::RoleResult).value<std::shared_ptr<sc::Result>>();

            auto sessionId = (*result)["session-id"].get_string();
            auto queryId = (*result)["query-id"].get_int();

            // mock scope should send session-id and query-id it received back via custom result's values
            QCOMPARE(sessionId, resultsView->sessionId());
            QCOMPARE(queryId, resultsView->queryId());
            QCOMPARE(queryId, 0);

            // new session id
            QVERIFY(sessionId != lastSessionId);

            lastSessionId = sessionId;
        }
    }

    void testResultMetadata()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("metadata");

        // get ResultsModel instance
        auto categories = resultsView->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ss::ResultsModelInterface*>());
        auto results = results_var.value<ss::ResultsModelInterface*>();
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleTitle).toString(), QString("result for: \"metadata\""));
        // mapped to the same field name
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleSubtitle).toString(), QString("subtitle"));
        // mapped to a different field name
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleEmblem).toString(), QString("emblem"));
        // mapped but not present in the result
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleMascot).toString(), QString());
        // unmapped
        QVERIFY(results->data(idx, ss::ResultsModelInterface::Roles::RoleAttributes).isNull());
        QVERIFY(results->data(idx, ss::ResultsModelInterface::Roles::RoleSummary).isNull());
    }

//    void testResultsInvalidation()
//    {
//        if (!QDBusConnection::sessionBus().isConnected()) {
//            QSKIP("DBus unavailable, skipping test");
//        }
//
//        QStringList args;
//        args << "/com/canonical/unity/scopes";
//        args << "com.canonical.unity.scopes.InvalidateResults";
//        args << "string:mock-scope";
//        QProcess::execute("dbus-send", args);
//
//        QSignalSpy spy(m_scope, SIGNAL(searchInProgressChanged()));
//        QCOMPARE(m_scope->searchInProgress(), false);
//        QVERIFY(spy.wait());
//        QCOMPARE(m_scope->searchInProgress(), true);
//        QVERIFY(spy.wait());
//        QCOMPARE(m_scope->searchInProgress(), false);
//    }

    void testActiveTtlScope()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope-ttl");
        resultsView->setQuery("query text");

        const QString query("query text");

        // get ResultsModel instance
        auto categories = resultsView->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0),
                ss::CategoriesInterface::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ss::ResultsModelInterface*>());
        auto results = results_var.value<ss::ResultsModelInterface*>();
        QVERIFY(results);
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);

        QVERIFY(results->data(idx, ss::ResultsModelInterface::Roles::RoleTitle).toString().startsWith("query text"));
//        QVERIFY(!m_scope_ttl->resultsDirty());

        // get the number appended to result title by scope (increased with every search by mock-scope-ttl).
        // this is required because whenever Scopes object is re-created with every test case from this file,
        // the search is executed automatically for all test scopes, and this affects internal counter of mock-ttl-scope
        // and values tested in this test case. The values are differnt if you run entire test suite or just select tests.
        auto resultCount = results->data(idx, ss::ResultsModelInterface::Roles::RoleTitle).toString().mid(query.length()).toInt();

        // The scope should refresh every 250 ms, and increment the query
        // counter each time.
        resultsView->waitForResultsChange();
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleTitle).toString(),
                QString("query text" + QString::number(++resultCount)));
//        QVERIFY(!m_scope_ttl->resultsDirty());

        resultsView->waitForResultsChange();
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleTitle).toString(),
                QString("query text" + QString::number(++resultCount)));
//        QVERIFY(!m_scope_ttl->resultsDirty());

        resultsView->waitForResultsChange();
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleTitle).toString(),
                QString("query text" + QString::number(++resultCount)));
//        QVERIFY(!m_scope_ttl->resultsDirty());
    }
//
//    void testInactiveTtlScope()
//    {
//        m_scope_ttl->setActive(false);
//        m_scope_ttl->setSearchQuery("banana");
//
//        // Model should go dirty
//        QTRY_VERIFY(m_scope_ttl->resultsDirty());
//    }
//
    void testAlbumArtResult()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("music");

        // get ResultsModel instance
        auto categories = resultsView->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ss::ResultsModelInterface*>());
        auto results = results_var.value<ss::ResultsModelInterface*>();
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleUri).toString(), QString("file:///tmp/foo.mp3"));
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleTitle).toString(), QString("result for: \"music\""));
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleArt).toString(), QString("image://albumart/artist=Foo&album=FooAlbum"));
    }

    void testCategoryOverride()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("metadata");

        // get ResultsModel instance
        auto categories = resultsView->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ss::ResultsModelInterface*>());
        auto results = results_var.value<ss::ResultsModelInterface*>();
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleTitle).toString(), QString("result for: \"metadata\""));
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleEmblem).toString(), QString("emblem"));
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleArt).toString(), QString("art"));

        // drop all components but title
        categories->overrideCategoryJson("cat1", R"({"schema-version": 1, "components": {"title": "title"}})");
        // check that the model no longer has the components
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleTitle).toString(), QString("result for: \"metadata\""));
        QVERIFY(results->data(idx, ss::ResultsModelInterface::Roles::RoleEmblem).isNull());
        QVERIFY(results->data(idx, ss::ResultsModelInterface::Roles::RoleArt).isNull());

        categories->overrideCategoryJson("cat1", R"({"schema-version": 1, "components": {"title": "title", "art": {"field": "art"}}})");
        // check that the model has the art
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleTitle).toString(), QString("result for: \"metadata\""));
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleArt).toString(), QString("art"));
    }

    void testSpecialCategory()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("");

        auto categories = resultsView->categories();
        QString rawTemplate(R"({"schema-version": 1, "template": {"category-layout": "special"}})");
        CountObject* countObject = new CountObject(categories);
        categories->addSpecialCategory("special", "Special", "", rawTemplate, countObject);

        // should have 2 categories now
        QCOMPARE(categories->rowCount(), 2);
        QCOMPARE(categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleCount).toInt(), 0);
        countObject->setCount(1);
        QCOMPARE(categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleCount).toInt(), 1);

        qRegisterMetaType<QVector<int>>();
        QSignalSpy spy(categories, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)));

        countObject->setCountAsync(13);
        QCOMPARE(categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleCount).toInt(), 1);
        QTRY_COMPARE(categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleCount).toInt(), 13);

        // expecting a few dataChanged signals, count should have changed
        bool countChanged = false;
        while (!spy.empty() && !countChanged) {
            QList<QVariant> arguments = spy.takeFirst();
            auto roles = arguments.at(2).value<QVector<int>>();
            countChanged |= roles.contains(ss::CategoriesInterface::Roles::RoleCount);
        }
        QCOMPARE(countChanged, true);
    }

    void testCategoryWithRating()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("rating");

        // get ResultsModel instance
        auto categories = resultsView->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ss::ResultsModelInterface*>());
        auto results = results_var.value<ss::ResultsModelInterface*>();
        QVERIFY(results);
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleTitle).toString(), QString("result for: \"rating\""));
        auto attributes = results->data(idx, ss::ResultsModelInterface::Roles::RoleAttributes).toList();
        QVERIFY(attributes.size() >= 1);
        QCOMPARE(attributes.at(0).toMap().value("value").toString(), QString("21 reviews"));
    }

    void testCategoryAttributeLimit()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("attributes");

        // get ResultsModel instance
        auto categories = resultsView->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ss::ResultsModelInterface*>());
        auto results = results_var.value<ss::ResultsModelInterface*>();
        QVERIFY(results);
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleTitle).toString(), QString("result for: \"attributes\""));
        auto attributes = results->data(idx, ss::ResultsModelInterface::Roles::RoleAttributes).toList();
        QVERIFY(attributes.size() == 3);
        QCOMPARE(attributes[0].toMap().value("value").toString(), QString("21 reviews"));
        QCOMPARE(attributes[1].toMap().value("value").toString(), QString("4 comments"));
        QCOMPARE(attributes[2].toMap().value("value").toString(), QString("28 stars"));
    }

    void testCategoryWithBackground()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("background");

        // get ResultsModel instance
        auto categories = resultsView->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant renderer_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleRenderer);
        QVariantMap renderer(renderer_var.toMap());
        QVERIFY(renderer.contains("card-background"));
        QVERIFY(renderer["card-background"].canConvert<QVariantMap>());
        QVariantMap cardBackground(renderer["card-background"].toMap());
        QCOMPARE(cardBackground["type"], QVariant(QString("color")));
        QCOMPARE(cardBackground["elements"], QVariant(QVariantList({QString("black")})));
        QVariant results_var = categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ss::ResultsModelInterface*>());
        auto results = results_var.value<ss::ResultsModelInterface*>();
        QVERIFY(results);
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleTitle).toString(), QString("result for: \"background\""));
        QVariant background(results->data(idx, ss::ResultsModelInterface::Roles::RoleBackground));
        QVERIFY(background.canConvert<QVariantMap>());
        QVariantMap map(background.toMap());
        QCOMPARE(map["type"], QVariant(QString("gradient")));
        QCOMPARE(map["elements"], QVariant(QVariantList({QString("green"), QString("#ff00aa33")})));
    }

    void testCategoryDefaults()
    {
        // this search return minimal category definition, defaults should kick in
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("minimal");

        auto categories = resultsView->categories();
        QVERIFY(categories->rowCount() > 0);

        // get renderer_template and components
        auto cidx = categories->index(0);
        QVariant components_var = categories->data(cidx, ss::CategoriesInterface::Roles::RoleComponents);
        QVERIFY(components_var.canConvert<QVariantMap>());
        QJsonObject components = QJsonValue::fromVariant(components_var).toObject();
        QVariant renderer_var = categories->data(cidx, ss::CategoriesInterface::Roles::RoleRenderer);
        QVERIFY(renderer_var.canConvert<QVariantMap>());
        QJsonObject renderer = QJsonValue::fromVariant(renderer_var).toObject();

        int num_active_components = 0;
        for (auto it = components.begin(); it != components.end(); ++it) {
            if (it.value().isObject() && it.value().toObject().value("field").isString()) {
                num_active_components++;
            }
        }
        QCOMPARE(num_active_components, 1);
        QVERIFY(renderer.contains("card-size"));
        QCOMPARE(renderer.value("card-size"), QJsonValue(QString("small")));
        QVERIFY(renderer.contains("card-layout"));
        QCOMPARE(renderer.value("card-layout"), QJsonValue(QString("vertical")));
        QVERIFY(renderer.contains("category-layout"));
        QCOMPARE(renderer.value("category-layout"), QJsonValue(QString("grid")));

        // get ResultsModel instance
        QVariant results_var = categories->data(cidx, ss::CategoriesInterface::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ss::ResultsModelInterface*>());
        auto results = results_var.value<ss::ResultsModelInterface*>();
        QVERIFY(results);
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleTitle).toString(), QString("result for: \"minimal\""));
        // components json doesn't specify "art"
        QCOMPARE(results->data(idx, ss::ResultsModelInterface::Roles::RoleArt).toString(), QString());
    }

    void testCategoryDefinitionChange()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("z");

        auto categories = resultsView->categories();
        QVERIFY(categories->rowCount() > 0);

        qRegisterMetaType<QVector<int>>();
        QSignalSpy spy(categories, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)));

        // should at least change components
        resultsView->setQuery("metadata");

        // expecting a few dataChanged signals, count and components changes
        // ensure we get the components one
        bool componentsChanged = false;
        while (!spy.empty() && !componentsChanged) {
            QList<QVariant> arguments = spy.takeFirst();
            auto roles = arguments.at(2).value<QVector<int>>();
            componentsChanged |= roles.contains(ss::CategoriesInterface::Roles::RoleComponents);
        }

        QCOMPARE(componentsChanged, true);
    }

    void testCategoryOrderChange()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("two-categories");

        auto categories = resultsView->categories();
        QCOMPARE(categories->rowCount(), 2);

        QStringList order1;
        order1 << categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleCategoryId).toString();
        order1 << categories->data(categories->index(1), ss::CategoriesInterface::Roles::RoleCategoryId).toString();

        resultsView->setQuery("two-categories-reversed");
        QCOMPARE(categories->rowCount(), 2);

        QStringList order2;
        order2 << categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleCategoryId).toString();
        order2 << categories->data(categories->index(1), ss::CategoriesInterface::Roles::RoleCategoryId).toString();

        QCOMPARE(order1[0], QString("cat1"));
        QCOMPARE(order1[1], QString("cat2"));
        QCOMPARE(order2[0], QString("cat2"));
        QCOMPARE(order2[1], QString("cat1"));
        QCOMPARE(order1[0], order2[1]);
        QCOMPARE(order1[1], order2[0]);
    }

    void testCategoryOrderChange2()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("two-categories-one-result");

        auto categories = resultsView->categories();
        QCOMPARE(categories->rowCount(), 1);

        QStringList order1;
        order1 << categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleCategoryId).toString();

        resultsView->setQuery("two-categories-reversed");
        QCOMPARE(categories->rowCount(), 2);

        QStringList order2;
        order2 << categories->data(categories->index(0), ss::CategoriesInterface::Roles::RoleCategoryId).toString();
        order2 << categories->data(categories->index(1), ss::CategoriesInterface::Roles::RoleCategoryId).toString();

        QCOMPARE(order1[0], QString("cat1"));
        QCOMPARE(order2[0], QString("cat2"));
        QCOMPARE(order2[1], QString("cat1"));
        QCOMPARE(order1[0], order2[1]);
    }

    void testScopeActivation()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("v");

        unity::scopes::Result::SPtr result;
        QVERIFY(sh::getFirstResult(resultsView->categories(), result));

        QSignalSpy spy(resultsView->activeScope(), SIGNAL(hideDash()));
        resultsView->activeScope()->activate(QVariant::fromValue(result));
        QVERIFY(spy.wait());
    }

    void testScopeActivationWithQuery()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("perform-query");

        unity::scopes::Result::SPtr result;
        QVERIFY(sh::getFirstResult(resultsView->activeScope()->categories(), result));

        QSignalSpy spy(resultsView->activeScope(), SIGNAL(gotoScope(QString)));
        resultsView->activeScope()->activate(QVariant::fromValue(result));
        QVERIFY(spy.wait());
    }

    void testScopeActivationWithQuery2()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("perform-query2");

        unity::scopes::Result::SPtr result;
        QVERIFY(sh::getFirstResult(resultsView->activeScope()->categories(), result));

        QSignalSpy spy(resultsView->activeScope(), SIGNAL(metadataRefreshed()));
        QSignalSpy spy2(resultsView->activeScope(), SIGNAL(gotoScope(QString)));
        QSignalSpy spy3(resultsView->activeScope(), SIGNAL(openScope(unity::shell::scopes::ScopeInterface*)));
        // this tries to activate non-existing scope
        resultsView->activeScope()->activate(QVariant::fromValue(result));
        QVERIFY(spy.wait());
        QCOMPARE(spy2.count(), 0);
        QCOMPARE(spy3.count(), 0);
    }

    void testScopeResultWithScopeUri()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("scope-uri");

        unity::scopes::Result::SPtr result;
        QVERIFY(sh::getFirstResult(resultsView->activeScope()->categories(), result));

        QSignalSpy spy(resultsView->activeScope(), SIGNAL(searchQueryChanged()));
        resultsView->activeScope()->activate(QVariant::fromValue(result));
        // this is likely to be invoked synchronously
        if (spy.count() == 0) {
            QVERIFY(spy.wait());
        }
        QVERIFY(spy.count() > 0);
        QCOMPARE(resultsView->activeScope()->searchQuery(), QString("next-scope-query"));
    }

    void testInfoStatus()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope-info");

        // No info (Status::Okay)
        resultsView->setQuery("no_info");
        QCOMPARE(resultsView->status(), ss::ScopeInterface::Status::Okay);
        // NoInternet (Status::NoInternet)
        resultsView->setQuery("no_internet");
        QCOMPARE(resultsView->status(), ss::ScopeInterface::Status::NoInternet);
        // NoLocationData (Status::NoLocationData)
        resultsView->setQuery("no_location");
        QCOMPARE(resultsView->status(), ss::ScopeInterface::Status::NoLocationData);
        // DefaultSettingsUsed (unknown to shell but known to run-time so Status::Okay)
        resultsView->setQuery("shell_unknown");
        QCOMPARE(resultsView->status(), ss::ScopeInterface::Status::Okay);
        // DefaultSettingsUsed (unknown to runtime so Status::Unknown)
        resultsView->setQuery("runtime_unknown");
        QCOMPARE(resultsView->status(), ss::ScopeInterface::Status::Unknown);
        // NoLocationData and NoInternet (Status::NoInternet takes priority)
        resultsView->setQuery("no_location_no_internet");
        QCOMPARE(resultsView->status(), ss::ScopeInterface::Status::NoInternet);
    }

};

QTEST_GUILESS_MAIN(ResultsTest)
#include <resultstest.moc>
