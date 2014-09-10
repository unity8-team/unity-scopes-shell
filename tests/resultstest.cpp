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

#include "registry-spawner.h"
#include "test-utils.h"

using namespace scopes_ng;

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
    QScopedPointer<Scopes> m_scopes;
    Scope* m_scope;
    Scope* m_scope_ttl;
    Scope* m_scope_info;
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
        m_scopes.reset(nullptr);

        const QStringList favs {"scope://mock-scope", "scope://mock-scope-ttl", "scope://mock-scope-info"};
        setFavouriteScopes(favs);

        m_scopes.reset(new Scopes(nullptr));
        // no scopes on startup
        QCOMPARE(m_scopes->rowCount(), 0);
        QCOMPARE(m_scopes->loaded(), false);
        QSignalSpy spy(m_scopes.data(), SIGNAL(loadedChanged()));
        // wait till the registry spawns
        QVERIFY(spy.wait());
        QCOMPARE(m_scopes->loaded(), true);
        // should have at least one scope now
        QVERIFY(m_scopes->rowCount() > 1);

        // get scope proxy
        m_scope = qobject_cast<scopes_ng::Scope*>(m_scopes->getScopeById(QString("mock-scope")));
        QVERIFY(m_scope != nullptr);
        m_scope->setActive(true);

        // get scope proxy for TTL scope
        m_scope_ttl = qobject_cast<scopes_ng::Scope*>(m_scopes->getScopeById(QString("mock-scope-ttl")));
        QVERIFY(m_scope_ttl != nullptr);
        m_scope_ttl->setActive(true);

        // get scope proxy for info scope (sends info() messages)
        m_scope_info = qobject_cast<scopes_ng::Scope*>(m_scopes->getScopeById(QString("mock-scope-info")));
        QVERIFY(m_scope_info != nullptr);
        m_scope_info->setActive(true);

        QTRY_COMPARE(m_scope_ttl->searchInProgress(), false);
        QTRY_COMPARE(m_scope->searchInProgress(), false);
        QTRY_COMPARE(m_scope_info->searchInProgress(), false);
    }

    void cleanup()
    {
        m_scopes.reset();
        m_scope = nullptr;
        m_scope_ttl = nullptr;
        m_scope_info = nullptr;
    }

    void testScopeCommunication()
    {
        // ensure categories have > 0 rows
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());
        QCOMPARE(categories->data(categories->index(0), Categories::Roles::RoleName), QVariant(QString("Category 1")));
        QCOMPARE(categories->data(categories->index(0), Categories::Roles::RoleIcon), QVariant(QString("")));
        QVERIFY(categories->data(categories->index(0), Categories::Roles::RoleHeaderLink).toString().isNull());

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

    void testScopeProperties()
    {
        QCOMPARE(m_scope->id(), QString("mock-scope"));
        QCOMPARE(m_scope->name(), QString("mock.DisplayName"));
        QCOMPARE(m_scope->iconHint(), QString("/mock.Icon"));
        QCOMPARE(m_scope->description(), QString("mock.Description"));
        QCOMPARE(m_scope->searchHint(), QString("mock.SearchHint"));
        QCOMPARE(m_scope->shortcut(), QString("mock.HotKey"));
        QCOMPARE(m_scope->searchQuery(), QString());

        QVariantMap customizations(m_scope->customizations());
        QVERIFY(customizations.size() > 0);
        QCOMPARE(static_cast<QMetaType::Type>(customizations["page-header"].type()), QMetaType::QVariantMap);
        QVariantMap headerCustomizations(customizations["page-header"].toMap());
        QCOMPARE(headerCustomizations["logo"], QVariant("http://assets.ubuntu.com/sites/ubuntu/1110/u/img/logos/logo-ubuntu-orange.svg"));
        QCOMPARE(headerCustomizations["foreground-color"], QVariant("white"));
        QCOMPARE(headerCustomizations["background"], QVariant("color://black"));
        QCOMPARE(customizations["shape-images"], QVariant(false));

        QCOMPARE(m_scope->isActive(), true);
        m_scope->setActive(false);
        QCOMPARE(m_scope->isActive(), false);

        QCOMPARE(m_scope_ttl->id(), QString("mock-scope-ttl"));
        QCOMPARE(m_scope_ttl->name(), QString("mock-ttl.DisplayName"));
        QCOMPARE(m_scope_ttl->iconHint(), QString("/mock-ttl.Icon"));
        QCOMPARE(m_scope_ttl->description(), QString("mock-ttl.Description"));
        QCOMPARE(m_scope_ttl->searchHint(), QString());
        QCOMPARE(m_scope_ttl->shortcut(), QString());
        QCOMPARE(m_scope_ttl->searchQuery(), QString());
    }

    void testCategoryQuery()
    {
        performSearch(m_scope, QString("expansion-query"));

        // ensure categories have > 0 rows
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());
        QCOMPARE(categories->data(categories->index(0), Categories::Roles::RoleName), QVariant(QString("Category 1")));
        QCOMPARE(categories->data(categories->index(0), Categories::Roles::RoleIcon), QVariant(QString("")));
        QVERIFY(categories->data(categories->index(0), Categories::Roles::RoleHeaderLink).toString().startsWith("scope://"));
    }

    void testTwoSearches()
    {
        // ensure categories have > 0 rows
        auto categories = m_scope->categories();
        auto categories_count = categories->rowCount();
        QVERIFY(categories_count > 0);

        performSearch(m_scope, QString("foo"));

        // shouldn't create more nor fewer categories
        QVERIFY(categories->rowCount() == categories_count);
    }

    void testBasicResultData()
    {
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

    void testSessionId()
    {
        std::string lastSessionId;

        performSearch(m_scope, QString(""));

        QVERIFY(!m_scope->sessionId().isEmpty());
        QCOMPARE(m_scope->queryId(), 0);

        {
            auto categories = m_scope->categories();
            QVERIFY(categories->rowCount() > 0);
            QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
            auto results = results_var.value<ResultsModel*>();
            QVERIFY(results->rowCount() > 0);

            auto idx = results->index(0);
            auto result = results->data(idx, ResultsModel::Roles::RoleResult).value<std::shared_ptr<unity::scopes::Result>>();

            auto sessionId = (*result)["session-id"].get_string();
            auto queryId = (*result)["query-id"].get_int();

            // mock scope should send session-id and query-id it received back via custom result's values
            QCOMPARE(sessionId, m_scope->sessionId().toStdString());
            QCOMPARE(queryId, m_scope->queryId());
            QCOMPARE(queryId, 0);

            lastSessionId = sessionId;
        }

        // new search
        performSearch(m_scope, QString("m"));
        {
            auto categories = m_scope->categories();
            QVERIFY(categories->rowCount() > 0);
            QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
            auto results = results_var.value<ResultsModel*>();
            QVERIFY(results->rowCount() > 0);

            auto idx = results->index(0);
            auto result = results->data(idx, ResultsModel::Roles::RoleResult).value<std::shared_ptr<unity::scopes::Result>>();

            auto sessionId = (*result)["session-id"].get_string();
            auto queryId = (*result)["query-id"].get_int();

            // mock scope should send session-id and query-id it received back via custom result's values
            QCOMPARE(sessionId, m_scope->sessionId().toStdString());
            QCOMPARE(queryId, m_scope->queryId());
            QCOMPARE(queryId, 0);

            // new session id
            QVERIFY(sessionId != lastSessionId);

            lastSessionId = sessionId;
        }

        // appends to previous search
        performSearch(m_scope, QString("met"));
        {
            auto categories = m_scope->categories();
            QVERIFY(categories->rowCount() > 0);
            QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
            auto results = results_var.value<ResultsModel*>();
            QVERIFY(results->rowCount() > 0);

            auto idx = results->index(0);
            auto result = results->data(idx, ResultsModel::Roles::RoleResult).value<std::shared_ptr<unity::scopes::Result>>();

            auto sessionId = (*result)["session-id"].get_string();
            auto queryId = (*result)["query-id"].get_int();

            // mock scope should send session-id and query-id it received back via custom result's values
            QCOMPARE(sessionId, m_scope->sessionId().toStdString());
            QCOMPARE(queryId, m_scope->queryId());
            QCOMPARE(queryId, 1);

            // session id unchanged
            QVERIFY(sessionId == lastSessionId);

            lastSessionId = sessionId;
        }

        // removes characters from previous search
        performSearch(m_scope, QString("me"));
        {
            auto categories = m_scope->categories();
            QVERIFY(categories->rowCount() > 0);
            QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
            auto results = results_var.value<ResultsModel*>();
            QVERIFY(results->rowCount() > 0);

            auto idx = results->index(0);
            auto result = results->data(idx, ResultsModel::Roles::RoleResult).value<std::shared_ptr<unity::scopes::Result>>();

            auto sessionId = (*result)["session-id"].get_string();
            auto queryId = (*result)["query-id"].get_int();

            // mock scope should send session-id and query-id it received back via custom result's values
            QCOMPARE(sessionId, m_scope->sessionId().toStdString());
            QCOMPARE(queryId, m_scope->queryId());
            QCOMPARE(queryId, 2);

            // session id unchanged
            QVERIFY(sessionId == lastSessionId);

            lastSessionId = sessionId;
        }

        // new non-empty search again
        performSearch(m_scope, QString("iron"));
        {
            auto categories = m_scope->categories();
            QVERIFY(categories->rowCount() > 0);
            QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
            auto results = results_var.value<ResultsModel*>();
            QVERIFY(results->rowCount() > 0);

            auto idx = results->index(0);
            auto result = results->data(idx, ResultsModel::Roles::RoleResult).value<std::shared_ptr<unity::scopes::Result>>();

            auto sessionId = (*result)["session-id"].get_string();
            auto queryId = (*result)["query-id"].get_int();

            // mock scope should send session-id and query-id it received back via custom result's values
            QCOMPARE(sessionId, m_scope->sessionId().toStdString());
            QCOMPARE(queryId, m_scope->queryId());
            QCOMPARE(queryId, 0);

            // new session id
            QVERIFY(sessionId != lastSessionId);

            lastSessionId = sessionId;
        }

        // new empty search again
        performSearch(m_scope, QString(""));
        {
            auto categories = m_scope->categories();
            QVERIFY(categories->rowCount() > 0);
            QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
            auto results = results_var.value<ResultsModel*>();
            QVERIFY(results->rowCount() > 0);

            auto idx = results->index(0);
            auto result = results->data(idx, ResultsModel::Roles::RoleResult).value<std::shared_ptr<unity::scopes::Result>>();

            auto sessionId = (*result)["session-id"].get_string();
            auto queryId = (*result)["query-id"].get_int();

            // mock scope should send session-id and query-id it received back via custom result's values
            QCOMPARE(sessionId, m_scope->sessionId().toStdString());
            QCOMPARE(queryId, m_scope->queryId());
            QCOMPARE(queryId, 0);

            // new session id
            QVERIFY(sessionId != lastSessionId);

            lastSessionId = sessionId;
        }
    }

    void testResultMetadata()
    {
        performSearch(m_scope, QString("metadata"));

        // get ResultsModel instance
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleTitle).toString(), QString("result for: \"metadata\""));
        // mapped to the same field name
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleSubtitle).toString(), QString("subtitle"));
        // mapped to a different field name
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleEmblem).toString(), QString("emblem"));
        // mapped but not present in the result
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleMascot).toString(), QString());
        // unmapped
        QVERIFY(results->data(idx, ResultsModel::Roles::RoleAttributes).isNull());
        QVERIFY(results->data(idx, ResultsModel::Roles::RoleSummary).isNull());
    }

    void testResultsInvalidation()
    {
        if (!QDBusConnection::sessionBus().isConnected()) {
            QSKIP("DBus unavailable, skipping test");
        }

        performSearch(m_scope, QString(""));

        QStringList args;
        args << "/com/canonical/unity/scopes";
        args << "com.canonical.unity.scopes.InvalidateResults";
        args << "string:mock-scope";
        QProcess::execute("dbus-send", args);

        QSignalSpy spy(m_scope, SIGNAL(searchInProgressChanged()));
        QCOMPARE(m_scope->searchInProgress(), false);
        QVERIFY(spy.wait());
        QCOMPARE(m_scope->searchInProgress(), true);
        QVERIFY(spy.wait());
        QCOMPARE(m_scope->searchInProgress(), false);
    }

    void testActiveTtlScope()
    {
        const QString query("query text");
        performSearch(m_scope_ttl, query);

        // get ResultsModel instance
        auto categories = m_scope_ttl->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0),
                Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);

        QVERIFY(results->data(idx, ResultsModel::Roles::RoleTitle).toString().startsWith("query text"));
        QVERIFY(!m_scope_ttl->resultsDirty());

        // get the number appended to result title by scope (increased with every search by mock-scope-ttl).
        // this is required because whenever Scopes object is re-created with every test case from this file,
        // the search is executed automatically for all test scopes, and this affects internal counter of mock-ttl-scope
        // and values tested in this test case. The values are differnt if you run entire test suite or just select tests.
        auto resultCount = results->data(idx, ResultsModel::Roles::RoleTitle).toString().mid(query.length()).toInt();

        // The scope should refresh every 250 ms, and increment the query
        // counter each time.
        waitForResultsChange(m_scope_ttl);
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleTitle).toString(),
                QString("query text" + QString::number(++resultCount)));
        QVERIFY(!m_scope_ttl->resultsDirty());

        waitForResultsChange(m_scope_ttl);
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleTitle).toString(),
                QString("query text" + QString::number(++resultCount)));
        QVERIFY(!m_scope_ttl->resultsDirty());

        waitForResultsChange(m_scope_ttl);
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleTitle).toString(),
                QString("query text" + QString::number(++resultCount)));
        QVERIFY(!m_scope_ttl->resultsDirty());
    }

    void testInactiveTtlScope()
    {
        m_scope_ttl->setActive(false);
        m_scope_ttl->setSearchQuery("banana");

        // Model should go dirty
        QTRY_VERIFY(m_scope_ttl->resultsDirty());
    }

    void testAlbumArtResult()
    {
        performSearch(m_scope, QString("music"));

        // get ResultsModel instance
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleUri).toString(), QString("file:///tmp/foo.mp3"));
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleTitle).toString(), QString("result for: \"music\""));
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleArt).toString(), QString("image://albumart/artist=Foo&album=FooAlbum"));
    }

    void testCategoryOverride()
    {
        performSearch(m_scope, QString("metadata"));

        // get ResultsModel instance
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleTitle).toString(), QString("result for: \"metadata\""));
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleEmblem).toString(), QString("emblem"));
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleArt).toString(), QString("art"));

        // drop all components but title
        categories->overrideCategoryJson("cat1", R"({"schema-version": 1, "components": {"title": "title"}})");
        // check that the model no longer has the components
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleTitle).toString(), QString("result for: \"metadata\""));
        QVERIFY(results->data(idx, ResultsModel::Roles::RoleEmblem).isNull());
        QVERIFY(results->data(idx, ResultsModel::Roles::RoleArt).isNull());

        categories->overrideCategoryJson("cat1", R"({"schema-version": 1, "components": {"title": "title", "art": {"field": "art"}}})");
        // check that the model has the art
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleTitle).toString(), QString("result for: \"metadata\""));
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleArt).toString(), QString("art"));
    }

    void testSpecialCategory()
    {
        QCOMPARE(m_scope->searchInProgress(), false);

        auto categories = m_scope->categories();
        QString rawTemplate(R"({"schema-version": 1, "template": {"category-layout": "special"}})");
        CountObject* countObject = new CountObject(m_scope);
        categories->addSpecialCategory("special", "Special", "", rawTemplate, countObject);

        // should have 2 categories now
        QCOMPARE(categories->rowCount(), 2);
        QCOMPARE(categories->data(categories->index(0), Categories::Roles::RoleCount).toInt(), 0);
        countObject->setCount(1);
        QCOMPARE(categories->data(categories->index(0), Categories::Roles::RoleCount).toInt(), 1);

        qRegisterMetaType<QVector<int>>();
        QSignalSpy spy(categories, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)));

        countObject->setCountAsync(13);
        QCOMPARE(categories->data(categories->index(0), Categories::Roles::RoleCount).toInt(), 1);
        QTRY_COMPARE(categories->data(categories->index(0), Categories::Roles::RoleCount).toInt(), 13);

        // expecting a few dataChanged signals, count should have changed
        bool countChanged = false;
        while (!spy.empty() && !countChanged) {
            QList<QVariant> arguments = spy.takeFirst();
            auto roles = arguments.at(2).value<QVector<int>>();
            countChanged |= roles.contains(Categories::Roles::RoleCount);
        }
        QCOMPARE(countChanged, true);
    }

    void testCategoryWithRating()
    {
        performSearch(m_scope, QString("rating"));

        // get ResultsModel instance
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleTitle).toString(), QString("result for: \"rating\""));
        auto attributes = results->data(idx, ResultsModel::Roles::RoleAttributes).toList();
        QVERIFY(attributes.size() >= 1);
        QCOMPARE(attributes[0].toMap().value("value").toString(), QString("21 reviews"));
    }

    void testCategoryAttributeLimit()
    {
        performSearch(m_scope, QString("attributes"));

        // get ResultsModel instance
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleTitle).toString(), QString("result for: \"attributes\""));
        auto attributes = results->data(idx, ResultsModel::Roles::RoleAttributes).toList();
        QVERIFY(attributes.size() == 3);
        QCOMPARE(attributes[0].toMap().value("value").toString(), QString("21 reviews"));
        QCOMPARE(attributes[1].toMap().value("value").toString(), QString("4 comments"));
        QCOMPARE(attributes[2].toMap().value("value").toString(), QString("28 stars"));
    }

    void testCategoryWithBackground()
    {
        performSearch(m_scope, QString("background"));

        // get ResultsModel instance
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant renderer_var = categories->data(categories->index(0), Categories::Roles::RoleRenderer);
        QVariantMap renderer(renderer_var.toMap());
        QVERIFY(renderer.contains("card-background"));
        QVERIFY(renderer["card-background"].canConvert<QVariantMap>());
        QVariantMap cardBackground(renderer["card-background"].toMap());
        QCOMPARE(cardBackground["type"], QVariant(QString("color")));
        QCOMPARE(cardBackground["elements"], QVariant(QVariantList({QString("black")})));
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleTitle).toString(), QString("result for: \"background\""));
        QVariant background(results->data(idx, ResultsModel::Roles::RoleBackground));
        QVERIFY(background.canConvert<QVariantMap>());
        QVariantMap map(background.toMap());
        QCOMPARE(map["type"], QVariant(QString("gradient")));
        QCOMPARE(map["elements"], QVariant(QVariantList({QString("green"), QString("#ff00aa33")})));
    }

    void testCategoryDefaults()
    {
        // this search return minimal category definition, defaults should kick in
        performSearch(m_scope, QString("minimal"));

        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);

        // get renderer_template and components
        auto cidx = categories->index(0);
        QVariant components_var = categories->data(cidx, Categories::Roles::RoleComponents);
        QVERIFY(components_var.canConvert<QVariantMap>());
        QJsonObject components = QJsonValue::fromVariant(components_var).toObject();
        QVariant renderer_var = categories->data(cidx, Categories::Roles::RoleRenderer);
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
        QVariant results_var = categories->data(cidx, Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleTitle).toString(), QString("result for: \"minimal\""));
        // components json doesn't specify "art"
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleArt).toString(), QString());
    }

    void testCategoryDefinitionChange()
    {
        performSearch(m_scope, QString("z"));

        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);

        qRegisterMetaType<QVector<int>>();
        QSignalSpy spy(categories, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)));

        // should at least change components
        performSearch(m_scope, QString("metadata"));

        // expecting a few dataChanged signals, count and components changes
        // ensure we get the components one
        bool componentsChanged = false;
        while (!spy.empty() && !componentsChanged) {
            QList<QVariant> arguments = spy.takeFirst();
            auto roles = arguments.at(2).value<QVector<int>>();
            componentsChanged |= roles.contains(Categories::Roles::RoleComponents);
        }

        QCOMPARE(componentsChanged, true);
    }

    void testCategoryOrderChange()
    {
        performSearch(m_scope, QString("two-categories"));

        auto categories = m_scope->categories();
        QCOMPARE(categories->rowCount(), 2);

        QStringList order1;
        order1 << categories->data(categories->index(0), Categories::Roles::RoleCategoryId).toString();
        order1 << categories->data(categories->index(1), Categories::Roles::RoleCategoryId).toString();

        performSearch(m_scope, QString("two-categories-reversed"));
        QCOMPARE(categories->rowCount(), 2);

        QStringList order2;
        order2 << categories->data(categories->index(0), Categories::Roles::RoleCategoryId).toString();
        order2 << categories->data(categories->index(1), Categories::Roles::RoleCategoryId).toString();

        QCOMPARE(order1[0], QString("cat1"));
        QCOMPARE(order1[1], QString("cat2"));
        QCOMPARE(order2[0], QString("cat2"));
        QCOMPARE(order2[1], QString("cat1"));
        QCOMPARE(order1[0], order2[1]);
        QCOMPARE(order1[1], order2[0]);
    }

    void testCategoryOrderChange2()
    {
        performSearch(m_scope, QString("two-categories-one-result"));

        auto categories = m_scope->categories();
        QCOMPARE(categories->rowCount(), 1);

        QStringList order1;
        order1 << categories->data(categories->index(0), Categories::Roles::RoleCategoryId).toString();

        performSearch(m_scope, QString("two-categories-reversed"));
        QCOMPARE(categories->rowCount(), 2);

        QStringList order2;
        order2 << categories->data(categories->index(0), Categories::Roles::RoleCategoryId).toString();
        order2 << categories->data(categories->index(1), Categories::Roles::RoleCategoryId).toString();

        QCOMPARE(order1[0], QString("cat1"));
        QCOMPARE(order2[0], QString("cat2"));
        QCOMPARE(order2[1], QString("cat1"));
        QCOMPARE(order1[0], order2[1]);
    }

    void testScopeActivation()
    {
        performSearch(m_scope, QString("v"));

        unity::scopes::Result::SPtr result;
        QVERIFY(getFirstResult(m_scope, result));

        QSignalSpy spy(m_scope, SIGNAL(hideDash()));
        m_scope->activate(QVariant::fromValue(result));
        QVERIFY(spy.wait());
    }

    void testScopeActivationWithQuery()
    {
        performSearch(m_scope, QString("perform-query"));

        unity::scopes::Result::SPtr result;
        QVERIFY(getFirstResult(m_scope, result));

        QSignalSpy spy(m_scope, SIGNAL(gotoScope(QString)));
        m_scope->activate(QVariant::fromValue(result));
        QVERIFY(spy.wait());
    }

    void testScopeActivationWithQuery2()
    {
        performSearch(m_scope, QString("perform-query2"));

        unity::scopes::Result::SPtr result;
        QVERIFY(getFirstResult(m_scope, result));

        QSignalSpy spy(m_scopes.data(), SIGNAL(metadataRefreshed()));
        QSignalSpy spy2(m_scope, SIGNAL(gotoScope(QString)));
        QSignalSpy spy3(m_scope, SIGNAL(openScope(unity::shell::scopes::ScopeInterface*)));
        // this tries to activate non-existing scope
        m_scope->activate(QVariant::fromValue(result));
        QVERIFY(spy.wait());
        QCOMPARE(spy2.count(), 0);
        QCOMPARE(spy3.count(), 0);
    }

    void testScopeResultWithScopeUri()
    {
        performSearch(m_scope, QString("scope-uri"));

        unity::scopes::Result::SPtr result;
        QVERIFY(getFirstResult(m_scope, result));

        QSignalSpy spy(m_scope, SIGNAL(searchQueryChanged()));
        m_scope->activate(QVariant::fromValue(result));
        // this is likely to be invoked synchronously
        if (spy.count() == 0) {
            QVERIFY(spy.wait());
        }
        QVERIFY(spy.count() > 0);
        QCOMPARE(m_scope->searchQuery(), QString("next-scope-query"));
    }

    void testInfoStatus()
    {
        // No info (Status::Okay)
        performSearch(m_scope_info, QString("no_info"));
        QCOMPARE(m_scope_info->status(), unity::shell::scopes::ScopeInterface::Status::Okay);
        // NoInternet (Status::NoInternet)
        performSearch(m_scope_info, QString("no_internet"));
        QCOMPARE(m_scope_info->status(), unity::shell::scopes::ScopeInterface::Status::NoInternet);
        // NoLocationData (Status::NoLocationData)
        performSearch(m_scope_info, QString("no_location"));
        QCOMPARE(m_scope_info->status(), unity::shell::scopes::ScopeInterface::Status::NoLocationData);
        // DefaultSettingsUsed (unknown to shell but known to run-time so Status::Okay)
        performSearch(m_scope_info, QString("shell_unknown"));
        QCOMPARE(m_scope_info->status(), unity::shell::scopes::ScopeInterface::Status::Okay);
        // DefaultSettingsUsed (unknown to runtime so Status::Unknown)
        performSearch(m_scope_info, QString("runtime_unknown"));
        QCOMPARE(m_scope_info->status(), unity::shell::scopes::ScopeInterface::Status::Unknown);
        // NoLocationData and NoInternet (Status::NoInternet takes priority)
        performSearch(m_scope_info, QString("no_location_no_internet"));
        QCOMPARE(m_scope_info->status(), unity::shell::scopes::ScopeInterface::Status::NoInternet);
    }

};

QTEST_GUILESS_MAIN(ResultsTest)
#include <resultstest.moc>
