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

#include <unity/scopes/Variant.h>
#include <unity/scopes/VariantBuilder.h>

#include <scope-harness/matcher/category-matcher.h>
#include <scope-harness/matcher/category-list-matcher.h>
#include <scope-harness/matcher/result-matcher.h>
#include <scope-harness/view/preview-view.h>
#include <scope-harness/scope-harness.h>
#include <scope-harness/test-utils.h>

using namespace std;
namespace sh = unity::scopeharness;
namespace shm = unity::scopeharness::matcher;
namespace shv = unity::scopeharness::view;
namespace sc = unity::scopes;
namespace ss = unity::shell::scopes;

#define QVERIFY_MATCHRESULT(statement) \
do {\
    auto result = (statement);\
    QVERIFY2(result.success(), result.concat_failures().c_str());\
} while (0)

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
        // ensure results have some data
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .hasAtLeast(1)
                .mode(shm::CategoryListMatcher::Mode::by_id)
                .category(shm::CategoryMatcher("cat1")
                    .title("Category 1")
                    .icon(string())
                    .headerLink(string())
                    .hasAtLeast(1)
                )
                .match(resultsView->categories())
        );
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
        auto categories = resultsView->raw_categories();
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

        // ensure categories has 1 row
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .hasExactly(1)
                .match(resultsView->categories())
        );

        resultsView->setQuery("foo");

        // shouldn't create more nor fewer categories
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .hasExactly(1)
                .match(resultsView->categories())
        );
    }

    void testBasicResultData()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("");

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .hasAtLeast(1)
                .mode(shm::CategoryListMatcher::Mode::by_id)
                .category(shm::CategoryMatcher("cat1")
                    .hasAtLeast(1)
                    .mode(shm::CategoryMatcher::Mode::by_uri)
                    .result(shm::ResultMatcher("test:uri")
                        .dndUri("test:dnd_uri")
                        .title("result for: \"\"")
                        .art("art")
                    )
                )
                .match(resultsView->categories())
        );
    }

    void testSessionId()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("");

        string lastSessionId;

        QVERIFY(!resultsView->sessionId().empty());
        QCOMPARE(resultsView->queryId(), 0);

        {
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .category(shm::CategoryMatcher("cat1")
                        .mode(shm::CategoryMatcher::Mode::by_uri)
                        .result(shm::ResultMatcher("test:uri")
                            .property("session-id", sc::Variant(resultsView->sessionId()))
                            .property("query-id", sc::Variant(0))
                        )
                    )
                    .match(resultsView->categories())
            );

            lastSessionId = resultsView->category("cat1").results().front()["session-id"].get_string();
        }

        // new search
        resultsView->setQuery("m");
        {
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .category(shm::CategoryMatcher("cat1")
                        .mode(shm::CategoryMatcher::Mode::by_uri)
                        .result(shm::ResultMatcher("test:uri")
                            .property("session-id", sc::Variant(resultsView->sessionId()))
                            .property("query-id", sc::Variant(0))
                        )
                    )
                    .match(resultsView->categories())
            );

            auto sessionId = resultsView->category("cat1").results().front()["session-id"].get_string();

            // new session id
            QVERIFY(sessionId != lastSessionId);
            lastSessionId = sessionId;
        }

        // appends to previous search
        resultsView->setQuery("met");
        {
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .category(shm::CategoryMatcher("cat1")
                        .mode(shm::CategoryMatcher::Mode::by_uri)
                        .result(shm::ResultMatcher("test:uri")
                            .property("session-id", sc::Variant(resultsView->sessionId()))
                            .property("query-id", sc::Variant(1))
                        )
                    )
                    .match(resultsView->categories())
            );

            auto sessionId = resultsView->category("cat1").results().front()["session-id"].get_string();

            // session id unchanged
            QCOMPARE(sessionId, lastSessionId);

            lastSessionId = sessionId;
        }

        // removes characters from previous search
        resultsView->setQuery("m");
        {
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .category(shm::CategoryMatcher("cat1")
                        .mode(shm::CategoryMatcher::Mode::by_uri)
                        .result(shm::ResultMatcher("test:uri")
                            .property("session-id", sc::Variant(resultsView->sessionId()))
                            .property("query-id", sc::Variant(2))
                        )
                    )
                    .match(resultsView->categories())
            );

            auto sessionId = resultsView->category("cat1").results().front()["session-id"].get_string();

            // session id unchanged
            QVERIFY(sessionId == lastSessionId);

            lastSessionId = sessionId;
        }

        // new non-empty search again
        resultsView->setQuery("iron");
        {
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .category(shm::CategoryMatcher("cat1")
                        .mode(shm::CategoryMatcher::Mode::by_uri)
                        .result(shm::ResultMatcher("test:uri")
                            .property("session-id", sc::Variant(resultsView->sessionId()))
                            .property("query-id", sc::Variant(0))
                        )
                    )
                    .match(resultsView->categories())
            );

            auto sessionId = resultsView->category("cat1").results().front()["session-id"].get_string();

            // new session id
            QVERIFY(sessionId != lastSessionId);

            lastSessionId = sessionId;
        }

        // new empty search again
        resultsView->setQuery("");
        {
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .category(shm::CategoryMatcher("cat1")
                        .mode(shm::CategoryMatcher::Mode::by_uri)
                        .result(shm::ResultMatcher("test:uri")
                            .property("session-id", sc::Variant(resultsView->sessionId()))
                            .property("query-id", sc::Variant(0))
                        )
                    )
                    .match(resultsView->categories())
            );

            auto sessionId = resultsView->category("cat1").results().front()["session-id"].get_string();

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

        // various fields have been mapped
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat1")
                    .mode(shm::CategoryMatcher::Mode::by_uri)
                    .result(shm::ResultMatcher("test:uri")
                        .title("result for: \"metadata\"")
                        .subtitle("subtitle")
                        .emblem("emblem")
                        .mascot(string())
                        .attributes(sc::Variant())
                        .summary(sc::Variant())
                    )
                )
                .match(resultsView->categories())
        );
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
        auto categories = resultsView->raw_categories();
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

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat1")
                    .mode(shm::CategoryMatcher::Mode::by_uri)
                    .result(shm::ResultMatcher("file:///tmp/foo.mp3")
                        .title("result for: \"music\"")
                        .art("image://albumart/artist=Foo&album=FooAlbum")
                    )
                )
                .match(resultsView->categories())
        );
    }

    void testCategoryOverride()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("metadata");

        auto categories = resultsView->categories();

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat1")
                    .result(shm::ResultMatcher("test:uri")
                        .title("result for: \"metadata\"")
                        .emblem("emblem")
                        .art("art")
                    )
                )
                .match(categories)
        );

        // drop all components but title
        resultsView->overrideCategoryJson("cat1", R"({"schema-version": 1, "components": {"title": "title"}})");
        // check that the model no longer has the components
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat1")
                    .result(shm::ResultMatcher("test:uri")
                        .title("result for: \"metadata\"")
                        .emblem(string())
                        .art(string())
                    )
                )
                .match(categories)
        );

        resultsView->overrideCategoryJson("cat1", R"({"schema-version": 1, "components": {"title": "title", "art": {"field": "art"}}})");
        // check that the model has the art
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat1")
                    .result(shm::ResultMatcher("test:uri")
                        .title("result for: \"metadata\"")
                        .emblem(string())
                        .art("art")
                    )
                )
                .match(categories)
        );
    }

    void testSpecialCategory()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("");

        auto categories = resultsView->raw_categories();
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

        sc::VariantBuilder builder;
        builder.add_tuple({{"value", sc::Variant("21 reviews")}});

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .mode(shm::CategoryListMatcher::Mode::by_id)
                .hasAtLeast(1)
                .category(shm::CategoryMatcher("cat1")
                    .mode(shm::CategoryMatcher::Mode::by_uri)
                    .hasAtLeast(1)
                    .result(shm::ResultMatcher("test:uri")
                        .title("result for: \"rating\"")
                        .attributes(builder.end())
                    )
                )
                .match(resultsView->categories())
        );
    }

    void testCategoryAttributeLimit()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("attributes");

        // Verify we only have 3 attributes
        sc::VariantBuilder builder;
        builder.add_tuple({{"value", sc::Variant("21 reviews")}});
        builder.add_tuple({{"value", sc::Variant("4 comments")}});
        builder.add_tuple({{"value", sc::Variant("28 stars")}});

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .mode(shm::CategoryListMatcher::Mode::by_id)
                .hasAtLeast(1)
                .category(shm::CategoryMatcher("cat1")
                    .mode(shm::CategoryMatcher::Mode::by_uri)
                    .hasAtLeast(1)
                    .result(shm::ResultMatcher("test:uri")
                        .title("result for: \"attributes\"")
                        .attributes(builder.end())
                    )
                )
                .match(resultsView->categories())
        );
    }

    void testCategoryWithBackground()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("background");

        sc::VariantMap renderer
        {
            {"card-background", sc::Variant(sc::VariantMap{
                {"elements", sc::Variant(sc::VariantArray{sc::Variant("black")})},
                {"type", sc::Variant("color")}
            })},
            {"card-layout", sc::Variant("vertical")},
            {"card-size", sc::Variant("small")},
            {"category-layout", sc::Variant("grid")},
            {"collapsed-rows", sc::Variant(2.0)},
            {"overlay-mode", sc::Variant()}
        };

        sc::VariantMap background
        {
            {"elements", sc::Variant(
                sc::VariantArray{
                    sc::Variant("green"),
                    sc::Variant("#ff00aa33")
                })
            },
            {"type", sc::Variant("gradient") }
        };

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat1")
                    .renderer(sc::Variant(renderer))
                    .result(shm::ResultMatcher("test:uri")
                        .title("result for: \"background\"")
                        .background(sc::Variant(background))
                    )
                )
                .match(resultsView->categories())
        );
    }

    void testCategoryDefaults()
    {
        // this search return minimal category definition, defaults should kick in
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("minimal");

        sc::VariantMap renderer
        {
            {"card-layout", sc::Variant("vertical")},
            {"card-size", sc::Variant("small")},
            {"category-layout", sc::Variant("grid")},
            {"collapsed-rows", sc::Variant(2.0)},
            {"overlay-mode", sc::Variant()}
        };

        sc::VariantMap components
        {
            {"art", sc::Variant(sc::VariantMap{{"aspect-ratio", sc::Variant(1.0)}})},
            {"attributes", sc::Variant(sc::VariantMap{{"max-count", sc::Variant(2.0)}})},
            {"background", sc::Variant()},
            {"emblem", sc::Variant()},
            {"mascot", sc::Variant()},
            {"overlay-color", sc::Variant()},
            {"subtitle", sc::Variant()},
            {"summary", sc::Variant()},
            {"title", sc::Variant(sc::VariantMap{{"field", sc::Variant("title")}})}
        };

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat1")
                    .renderer(sc::Variant(renderer))
                    .components(sc::Variant(components))
                    .result(shm::ResultMatcher("test:uri")
                        .title("result for: \"minimal\"")
                        .art(string())
                    )
                )
                .match(resultsView->categories())
        );
    }

    void testCategoryDefinitionChange()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("z");

        auto categories = resultsView->raw_categories();
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

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat1"))
                .category(shm::CategoryMatcher("cat2"))
                .match(resultsView->categories())
        );

        resultsView->setQuery("two-categories-reversed");
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat2"))
                .category(shm::CategoryMatcher("cat1"))
                .match(resultsView->categories())
        );
    }

    void testCategoryOrderChange2()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");

        resultsView->setQuery("two-categories-one-result");

        // FIXME: There are actually 3 categories in the CategoryModel at this point
        // It seems like categories aren't being cleared out (perhaps intentionally?)
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat1")
                    .result(shm::ResultMatcher("test:uri"))
                )
                .category(shm::CategoryMatcher("cat2"))
                .match(resultsView->categories())
        );

        resultsView->setQuery("two-categories-reversed");
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat2"))
                .category(shm::CategoryMatcher("cat1"))
                .match(resultsView->categories())
        );
    }

    /**
     * This test activates a result which is previewed normally
     */
    void testScopeActivation()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("v");


        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .mode(shm::CategoryListMatcher::Mode::starts_with)
                .category(shm::CategoryMatcher("cat1")
                    .mode(shm::CategoryMatcher::Mode::starts_with)
                    .result(shm::ResultMatcher("test:uri"))
                )
                .match(resultsView->categories())
        );

        auto abstractView =
                resultsView->category("cat1").result("test:uri").activate();
        QVERIFY(bool(abstractView));
        auto previewView = dynamic_pointer_cast<shv::PreviewView>(abstractView);
        QVERIFY(bool(previewView));
    }

    /**
     * This test activates a result that points to mock-scope-ttl
     */
    void testScopeActivationWithQuery()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("perform-query");

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .mode(shm::CategoryListMatcher::Mode::starts_with)
                .category(shm::CategoryMatcher("cat1")
                    .mode(shm::CategoryMatcher::Mode::starts_with)
                    .result(shm::ResultMatcher("test:perform-query"))
                )
                .match(resultsView->categories())
        );

        auto abstractView =
                resultsView->category("cat1").result("test:perform-query").activate();
        QVERIFY(bool(abstractView));
        auto nextView = dynamic_pointer_cast<shv::ResultsView>(abstractView);
        QVERIFY(bool(nextView));

        QCOMPARE(nextView->scopeId(), string("mock-scope-ttl"));
    }

    /**
     * This test tries to activate a result that points to a non-existing scope
     */
    void testScopeActivationWithQuery2()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("perform-query2");

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .mode(shm::CategoryListMatcher::Mode::starts_with)
                .category(shm::CategoryMatcher("cat1")
                    .mode(shm::CategoryMatcher::Mode::starts_with)
                    .result(shm::ResultMatcher("test:perform-query"))
                )
                .match(resultsView->categories())
        );

        auto abstractView =
                resultsView->category("cat1").result("test:perform-query").activate();
        QVERIFY(bool(abstractView));
        auto nextView = dynamic_pointer_cast<shv::ResultsView>(abstractView);
        QVERIFY(bool(nextView));

        // We shouldn't have gone anywhere
        QCOMPARE(nextView->scopeId(), string("mock-scope"));
    }

    void testScopeResultWithScopeUri()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("scope-uri");

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .mode(shm::CategoryListMatcher::Mode::starts_with)
                .category(shm::CategoryMatcher("cat1")
                    .mode(shm::CategoryMatcher::Mode::starts_with)
                    .result(shm::ResultMatcher("scope://mock-scope?q=next-scope-query"))
                )
                .match(resultsView->categories())
        );

        auto abstractView =
                resultsView->category("cat1").result("scope://mock-scope?q=next-scope-query").activate();
        QVERIFY(bool(abstractView));
        auto nextView = dynamic_pointer_cast<shv::ResultsView>(abstractView);
        QVERIFY(bool(nextView));

        QCOMPARE(resultsView->searchQuery(), string("next-scope-query"));
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .mode(shm::CategoryListMatcher::Mode::starts_with)
                .category(shm::CategoryMatcher("cat1")
                    .mode(shm::CategoryMatcher::Mode::starts_with)
                    .result(shm::ResultMatcher("next-scope-query")
                        .title("result for: \"next-scope-query\"")
                        .art("next-scope-query-art")
                    )
                )
                .match(resultsView->categories())
        );
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
