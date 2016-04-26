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
#include <QTimer>
#include <QSignalSpy>
#include <QDBusConnection>
#include <QDebug>

#include <chrono>
#include <cstdlib>
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

using namespace std;
namespace sh = unity::scopeharness;
namespace shm = unity::scopeharness::matcher;
namespace shr = unity::scopeharness::registry;
namespace shv = unity::scopeharness::view;
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
        qputenv("UNITY_SCOPES_CARDINALITY_OVERRIDE", "9999");
        m_harness = sh::ScopeHarness::newFromScopeList(
            shr::CustomRegistry::Parameters({
                TEST_DATA_DIR "mock-scope/mock-scope.ini",
                TEST_DATA_DIR "mock-scope-info/mock-scope-info.ini",
                TEST_DATA_DIR "mock-scope-ttl/mock-scope-ttl.ini",
                TEST_DATA_DIR "mock-scope-manyresults/mock-scope-manyresults.ini"
            })
        );
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
        QVERIFY(resultsView->query() == "");

        sc::VariantMap customizations(resultsView->customizations().get_dict());
        QVERIFY(!customizations.empty());
        QCOMPARE(customizations["page-header"].which(), sc::Variant::Type::Dict);
        sc::VariantMap headerCustomizations(customizations["page-header"].get_dict());
        QCOMPARE(headerCustomizations["logo"], sc::Variant("http://assets.ubuntu.com/sites/ubuntu/1110/u/img/logos/logo-ubuntu-orange.svg"));
        QCOMPARE(headerCustomizations["foreground-color"], sc::Variant("white"));
        QCOMPARE(headerCustomizations["background"], sc::Variant("color://black"));
        QCOMPARE(customizations["shape-images"], sc::Variant(false));

        resultsView->setActiveScope("mock-scope-ttl");

        QVERIFY(resultsView->scopeId() == "mock-scope-ttl");
        QVERIFY(resultsView->displayName() == "mock-ttl.DisplayName");
        QVERIFY(resultsView->iconHint() == "/mock-ttl.Icon");
        QVERIFY(resultsView->description() == "mock-ttl.Description");
        QVERIFY(resultsView->searchHint() == "");
        QVERIFY(resultsView->shortcut() == "");
        QVERIFY(resultsView->query() == "");
    }

    void testCategoryQuery()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("expansion-query");

        // ensure categories have > 0 rows
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat1")
                    .title("Category 1")
                    .icon("")
                    .headerLink("scope://mock-scope?q=expansion%2Dquery")
                    .mode(shm::CategoryMatcher::Mode::by_uri)
                    .hasAtLeast(1)
                )
                .match(resultsView->categories())
        );
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
                        .property("booleanness", sc::Variant(true))
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

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat1")
                    .mode(shm::CategoryMatcher::Mode::by_uri)
                    .result(shm::ResultMatcher("test:uri")
                        .title("query text2")
                    )
                )
                .match(resultsView->categories())
        );

        // The scope should refresh every 250 ms, and increment the query
        // counter each time.
        resultsView->waitForResultsChange();
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat1")
                    .mode(shm::CategoryMatcher::Mode::by_uri)
                    .result(shm::ResultMatcher("test:uri")
                        .title("query text3")
                    )
                )
                .match(resultsView->categories())
        );

        resultsView->waitForResultsChange();
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat1")
                    .mode(shm::CategoryMatcher::Mode::by_uri)
                    .result(shm::ResultMatcher("test:uri")
                        .title("query text4")
                    )
                )
                .match(resultsView->categories())
        );

        resultsView->waitForResultsChange();
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat1")
                    .mode(shm::CategoryMatcher::Mode::by_uri)
                    .result(shm::ResultMatcher("test:uri")
                        .title("query text5")
                    )
                )
                .match(resultsView->categories())
        );
    }

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
            {"social-actions", sc::Variant()},
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

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .hasAtLeast(1)
                .match(resultsView->categories())
        );

        // FIXME Restore category definition change test
//        qRegisterMetaType<QVector<int>>();
//        QSignalSpy spy(categories, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)));
//
//        // should at least change components
//        resultsView->setQuery("metadata");
//
//        // expecting a few dataChanged signals, count and components changes
//        // ensure we get the components one
//        bool componentsChanged = false;
//        while (!spy.empty() && !componentsChanged) {
//            QList<QVariant> arguments = spy.takeFirst();
//            auto roles = arguments.at(2).value<QVector<int>>();
//            componentsChanged |= roles.contains(ss::CategoriesInterface::Roles::RoleComponents);
//        }
//
//        QCOMPARE(componentsChanged, true);
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

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .category(shm::CategoryMatcher("cat1")
                    .result(shm::ResultMatcher("test:uri"))
                )
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
                resultsView->category("cat1").result("test:uri").longPress();
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
                    .result(shm::ResultMatcher("scope://test:perform-query"))
                )
                .match(resultsView->categories())
        );

        auto abstractView =
                resultsView->category("cat1").result("scope://test:perform-query").tap();
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
                    .result(shm::ResultMatcher("scope://test:perform-query"))
                )
                .match(resultsView->categories())
        );

        auto abstractView =
                resultsView->category("cat1").result("scope://test:perform-query").tap();
        QVERIFY(bool(abstractView));
        auto nextView = dynamic_pointer_cast<shv::ResultsView>(abstractView);
        QVERIFY(bool(nextView));

        // We shouldn't have gone anywhere
        QCOMPARE(nextView->scopeId(), string("mock-scope"));
    }

    /**
     * This test activates a result action
     */
    void testScopeResultActionActivation()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope");
        resultsView->setQuery("result-action");

        // activate action1
        {
            auto view = resultsView->category("cat1").result("test:result-action").tapAction("action1");
            QVERIFY(bool(view));
            auto nextView = dynamic_pointer_cast<shv::ResultsView>(view);
            QVERIFY(bool(nextView));
            QCOMPARE(resultsView, nextView);

            // check that mock scope updated the result by inserting 'actionId' in it
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .mode(shm::CategoryListMatcher::Mode::starts_with)
                    .category(shm::CategoryMatcher("cat1")
                        .mode(shm::CategoryMatcher::Mode::starts_with)
                        .result(
                            shm::ResultMatcher("test:result-action")
                                .property("actionId", sc::Variant("action1"))
                            )
                    )
                    .match(resultsView->categories())
            );
        }

        // activate action2
        {
            auto view = resultsView->category("cat1").result("test:result-action").tapAction("action2");
            QVERIFY(bool(view));
            auto nextView = dynamic_pointer_cast<shv::ResultsView>(view);
            QVERIFY(bool(nextView));
            QCOMPARE(resultsView, nextView);

            // check that mock scope updated the result
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .mode(shm::CategoryListMatcher::Mode::starts_with)
                    .category(shm::CategoryMatcher("cat1")
                        .mode(shm::CategoryMatcher::Mode::starts_with)
                        .result(
                            shm::ResultMatcher("test:result-action")
                                .property("actionId", sc::Variant("action2"))
                            )
                    )
                    .match(resultsView->categories())
            );
        }
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
                    .result(shm::ResultMatcher(shm::ScopeUri("mock-scope").query("next-scope-query")))
                )
                .match(resultsView->categories())
        );

        auto abstractView =
                resultsView->category("cat1").result(
                        shm::ScopeUri("mock-scope").query("next-scope-query").toString()).tap();
        QVERIFY(bool(abstractView));
        auto nextView = dynamic_pointer_cast<shv::ResultsView>(abstractView);
        QVERIFY(bool(nextView));

        QCOMPARE(resultsView->query(), string("next-scope-query"));
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

    void testResultsModelChanges()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope-manyresults");
        resultsView->setQuery("search1");
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .hasExactly(1)
                .category(shm::CategoryMatcher("cat1")
                    .result(shm::ResultMatcher("cat1_uri0"))
                    .result(shm::ResultMatcher("cat1_uri1"))
                    .result(shm::ResultMatcher("cat1_uri2"))
                    .result(shm::ResultMatcher("cat1_uri3"))
                    .result(shm::ResultMatcher("cat1_uri4"))
                )
                .match(resultsView->categories())
        );

        resultsView->setQuery("search2");
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .hasExactly(2)
                .category(shm::CategoryMatcher("cat1")
                    .result(shm::ResultMatcher("cat1_uri3"))
                    .result(shm::ResultMatcher("cat1_uri4"))
                    .result(shm::ResultMatcher("cat1_uri5"))
                    .result(shm::ResultMatcher("cat1_uri6"))
                    .result(shm::ResultMatcher("cat1_uri7"))
                )
                .category(shm::CategoryMatcher("cat2")
                    .result(shm::ResultMatcher("cat2_uri3"))
                    .result(shm::ResultMatcher("cat2_uri4"))
                    .result(shm::ResultMatcher("cat2_uri5"))
                    .result(shm::ResultMatcher("cat2_uri6"))
                    .result(shm::ResultMatcher("cat2_uri7"))
                )
                .match(resultsView->categories())
        );

        resultsView->setQuery("search3");
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .hasExactly(2)
                .category(shm::CategoryMatcher("cat1")
                    .result(shm::ResultMatcher("cat1_uri7"))
                    .result(shm::ResultMatcher("cat1_uri6"))
                    .result(shm::ResultMatcher("cat1_uri5"))
                    .result(shm::ResultMatcher("cat1_uri4"))
                    .result(shm::ResultMatcher("cat1_uri3"))
                )
                .category(shm::CategoryMatcher("cat2")
                    .result(shm::ResultMatcher("cat2_uri7"))
                    .result(shm::ResultMatcher("cat2_uri6"))
                    .result(shm::ResultMatcher("cat2_uri5"))
                    .result(shm::ResultMatcher("cat2_uri4"))
                    .result(shm::ResultMatcher("cat2_uri3"))
                )
                .match(resultsView->categories())
        );

        resultsView->setQuery("search4");
        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .hasExactly(1)
                .category(shm::CategoryMatcher("cat2")
                    .result(shm::ResultMatcher("cat2_uri5"))
                )
                .match(resultsView->categories())
        );
    }

    void testResultsModelChangesWithDuplicatedUris()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope-manyresults");

        // first search run
        {
            resultsView->setQuery("duplicated_uris1");
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .hasExactly(1)
                    .category(shm::CategoryMatcher("cat1")
                        .hasAtLeast(10)
                    )
                    .match(resultsView->categories())
            );

            auto const results = resultsView->category("cat1").results();
            QCOMPARE(static_cast<unsigned long>(results.size()), 10UL);
            for (unsigned i = 0; i<results.size(); i++) {
                QCOMPARE(
                        QString::fromStdString(results[i].uri()),
                        QString::fromStdString("uri")
                        );
            }
        }

        // second search run
        {
            resultsView->setQuery("duplicated_uris2");
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .hasExactly(1)
                    .category(shm::CategoryMatcher("cat1")
                        .hasAtLeast(10)
                    )
                    .match(resultsView->categories())
            );

            auto const results = resultsView->category("cat1").results();
            QCOMPARE(static_cast<unsigned long>(results.size()), 10UL);
            for (unsigned i = 0; i<results.size(); i++) {
                QCOMPARE(
                        QString::fromStdString(results[i].uri()),
                        (i % 2 == 0) ? QString::fromStdString("uri") : QString::fromStdString("uri" + std::to_string(i))
                        );
            }
        }
    }

    void testResultsModelChangesWithDuplicatedResults()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope-manyresults");

        {
            resultsView->setQuery("duplicated_results");
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .hasExactly(1)
                    .category(shm::CategoryMatcher("cat1")
                        .hasAtLeast(1)
                    )
                    .match(resultsView->categories())
            );

            auto const results = resultsView->category("cat1").results();
            QCOMPARE(static_cast<unsigned long>(results.size()), 1UL);
        }
    }

    void testResultsMassiveModelChanges()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope-manyresults");

        // first search run
        {
            auto const start = std::chrono::system_clock::now();

            resultsView->setQuery("lots_of_results");
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .hasExactly(1)
                    .category(shm::CategoryMatcher("cat1")
                        .hasAtLeast(2000)
                    )
                    .match(resultsView->categories())
            );
            auto end = std::chrono::system_clock::now();
            auto search_dur = std::chrono::duration_cast<std::chrono::seconds>(end.time_since_epoch()).count() - std::chrono::duration_cast<std::chrono::seconds>(start.time_since_epoch()).count();
            qDebug() << "Search #1 duration: " << search_dur;

            auto const results = resultsView->category("cat1").results();
            QCOMPARE(static_cast<unsigned long>(results.size()), 2000UL);
            for (unsigned i = 0; i<results.size(); i++) {
                QCOMPARE(
                        QString::fromStdString(results[i].uri()),
                        QString::fromStdString("cat1_uri" + std::to_string(i))
                        );
            }
        }

        // second search run, reversed order of results
        {
            auto const start = std::chrono::system_clock::now();

            resultsView->setQuery("lots_of_results_reversed_plus_some");
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .hasExactly(1)
                    .category(shm::CategoryMatcher("cat1")
                        .hasAtLeast(2100)
                    )
                    .match(resultsView->categories())
            );

            auto const end = std::chrono::system_clock::now();
            auto const search_dur = std::chrono::duration_cast<std::chrono::seconds>(end.time_since_epoch()).count() - std::chrono::duration_cast<std::chrono::seconds>(start.time_since_epoch()).count();
            qDebug() << "Search #2 duration: " << search_dur;

            auto const results = resultsView->category("cat1").results();
            QCOMPARE(static_cast<unsigned long>(results.size()), 2100UL);
            for (unsigned i = 0; i<results.size(); i++) {
                QCOMPARE(
                        QString::fromStdString(results[i].uri()),
                        QString::fromStdString("cat1_uri" + std::to_string(2099-i))
                        );
            }
        }

        // second search run, reversed order of results
        {
            auto const start = std::chrono::system_clock::now();

            resultsView->setQuery("lots_of_results_reversed");
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .hasExactly(1)
                    .category(shm::CategoryMatcher("cat1")
                        .hasAtLeast(2000)
                    )
                    .match(resultsView->categories())
            );

            auto const end = std::chrono::system_clock::now();
            auto const search_dur = std::chrono::duration_cast<std::chrono::seconds>(end.time_since_epoch()).count() - std::chrono::duration_cast<std::chrono::seconds>(start.time_since_epoch()).count();
            qDebug() << "Search #3 duration: " << search_dur;

            auto const results = resultsView->category("cat1").results();
            QCOMPARE(static_cast<unsigned long>(results.size()), 2000UL);
            for (unsigned i = 0; i<results.size(); i++) {
                QCOMPARE(
                        QString::fromStdString(results[i].uri()),
                        QString::fromStdString("cat1_uri" + std::to_string(1999-i))
                        );
            }
        }

        // 1000 results, every other matches previous set
        {
            auto const start = std::chrono::system_clock::now();

            resultsView->setQuery("lots_of_results_half_of_them_missing");
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .hasExactly(1)
                    .category(shm::CategoryMatcher("cat1")
                        .hasAtLeast(1000)
                    )
                    .match(resultsView->categories())
            );

            auto const end = std::chrono::system_clock::now();
            auto const search_dur = std::chrono::duration_cast<std::chrono::seconds>(end.time_since_epoch()).count() - std::chrono::duration_cast<std::chrono::seconds>(start.time_since_epoch()).count();
            qDebug() << "Search #4 duration: " << search_dur;

            auto const results = resultsView->category("cat1").results();
            QCOMPARE(static_cast<unsigned long>(results.size()), 1000UL);
            for (unsigned i = 0; i<results.size(); i++) {
                QCOMPARE(
                        QString::fromStdString(results[i].uri()),
                        QString::fromStdString("cat1_uri" + std::to_string(i*2))
                        );
            }
        }

        // third search run, different result set
        {
            auto const start = std::chrono::system_clock::now();

            resultsView->setQuery("lots_of_results_2");
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .hasExactly(1)
                    .category(shm::CategoryMatcher("cat1")
                        .hasAtLeast(2000)
                    )
                    .match(resultsView->categories())
            );

            auto const end = std::chrono::system_clock::now();
            auto const search_dur = std::chrono::duration_cast<std::chrono::seconds>(end.time_since_epoch()).count() - std::chrono::duration_cast<std::chrono::seconds>(start.time_since_epoch()).count();
            qDebug() << "Search #5 duration: " << search_dur;

            auto const results = resultsView->category("cat1").results();
            QCOMPARE(static_cast<unsigned long>(results.size()), 2000UL);
            for (unsigned i = 0; i<results.size(); i++) {
                QCOMPARE(
                        QString::fromStdString(results[i].uri()),
                        QString::fromStdString("cat1_uri" + std::to_string(5000+i))
                        );
            }
        }

        // fourth search run, no delays in the scope
        {
            auto const start = std::chrono::system_clock::now();

            resultsView->setQuery("lots_of_results_fast");
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .hasExactly(1)
                    .category(shm::CategoryMatcher("cat1")
                        .hasAtLeast(10)
                    )
                    .match(resultsView->categories())
            );
            auto end = std::chrono::system_clock::now();
            auto search_dur = std::chrono::duration_cast<std::chrono::seconds>(end.time_since_epoch()).count() - std::chrono::duration_cast<std::chrono::seconds>(start.time_since_epoch()).count();
            qDebug() << "Search #6 duration: " << search_dur;

            auto const results = resultsView->category("cat1").results();
            QCOMPARE(static_cast<unsigned long>(results.size()), 10UL);
            for (unsigned i = 0; i<results.size(); i++) {
                QCOMPARE(
                        QString::fromStdString(results[i].uri()),
                        QString::fromStdString("cat1_uri" + std::to_string(i))
                        );
            }
        }

        // fifth search run, reversed order of results, no delays in the scope
        {
            auto const start = std::chrono::system_clock::now();

            resultsView->setQuery("lots_of_results_reversed_fast");
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .hasExactly(1)
                    .category(shm::CategoryMatcher("cat1")
                        .hasAtLeast(10)
                    )
                    .match(resultsView->categories())
            );

            auto const end = std::chrono::system_clock::now();
            auto const search_dur = std::chrono::duration_cast<std::chrono::seconds>(end.time_since_epoch()).count() - std::chrono::duration_cast<std::chrono::seconds>(start.time_since_epoch()).count();
            qDebug() << "Search #7 duration: " << search_dur;

            auto const results = resultsView->category("cat1").results();
            QCOMPARE(static_cast<unsigned long>(results.size()), 10UL);
            for (unsigned i = 0; i<results.size(); i++) {
                QCOMPARE(
                        QString::fromStdString(results[i].uri()),
                        QString::fromStdString("cat1_uri" + std::to_string(9-i))
                        );
            }
        }
        // search with empty string
        {
            auto const start = std::chrono::system_clock::now();

            resultsView->setQuery("");
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .hasExactly(1)
                    .category(shm::CategoryMatcher("cat1")
                        .hasAtLeast(200)
                    )
                    .match(resultsView->categories())
            );

            auto const end = std::chrono::system_clock::now();
            auto const search_dur = std::chrono::duration_cast<std::chrono::seconds>(end.time_since_epoch()).count() - std::chrono::duration_cast<std::chrono::seconds>(start.time_since_epoch()).count();
            qDebug() << "Search #8 duration: " << search_dur;

            auto const results = resultsView->category("cat1").results();
            QCOMPARE(static_cast<unsigned long>(results.size()), 200UL);
            for (unsigned i = 0; i<results.size(); i++) {
                QCOMPARE(
                        QString::fromStdString(results[i].uri()),
                        QString::fromStdString("cat1_uri" + std::to_string(i))
                        );
            }
        }

    }

    void testResultsModelUpdatesRandomSearches()
    {
        // the aim of this test is to ensure no crashes; results art random and not verified
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope-manyresults");

        for (int i = 0; i<10; i++)
        {
            const unsigned long n = 1 + rand() % 100; // up to 100 results

            resultsView->setQuery("random" + std::to_string(n));
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .hasExactly(1)
                    .category(shm::CategoryMatcher("cat1")
                        .hasAtLeast(n)
                    )
                    .match(resultsView->categories())
            );

            auto const results = resultsView->category("cat1").results();
            QCOMPARE(static_cast<unsigned long>(results.size()), n);
        }
    }

    void testResultsModelUpdatesTwoCategories()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope-manyresults");

        {
            resultsView->setQuery("two-categories");
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .hasExactly(2)
                    .category(shm::CategoryMatcher("cat1")
                        .hasAtLeast(10)
                    )
                    .category(shm::CategoryMatcher("cat3")
                        .hasAtLeast(10)
                    )
                    .match(resultsView->categories())
            );

            auto const results1 = resultsView->category("cat1").results();
            QCOMPARE(static_cast<unsigned long>(results1.size()), 10UL);

            auto const results2 = resultsView->category("cat3").results();
            QCOMPARE(static_cast<unsigned long>(results2.size()), 10UL);
        }
        {
            resultsView->setQuery("two-categories-second-gone");
            QVERIFY_MATCHRESULT(
                shm::CategoryListMatcher()
                    .hasExactly(1)
                    .category(shm::CategoryMatcher("cat1")
                        .hasAtLeast(10)
                    )
                    .match(resultsView->categories())
            );

            auto const results1 = resultsView->category("cat1").results();
            QCOMPARE(static_cast<unsigned long>(results1.size()), 10UL);
        }
    }
};

QTEST_GUILESS_MAIN(ResultsTest)
#include <resultstest.moc>
