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
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonValue>
#include <QJsonObject>
#include <QProcess>
#include <QThread>
#include <QScopedPointer>
#include <QSignalSpy>

#include <scopes-ng/scopes.h>
#include <scopes-ng/scope.h>
#include <scopes-ng/categories.h>
#include <scopes-ng/resultsmodel.h>
#include <scopes-ng/previewmodel.h>
#include <scopes-ng/previewstack.h>
#include <scopes-ng/previewwidgetmodel.h>

#define SCOPES_TMP_ENDPOINT_DIR "/tmp/scopes-test-endpoints"

using namespace scopes_ng;

class CountObject : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit CountObject(QObject* parent = nullptr) : QObject(parent), m_count(0)
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

class ResultsTestNg : public QObject
{
    Q_OBJECT
private:
    QScopedPointer<Scopes> m_scopes;
    Scope* m_scope;
    QScopedPointer<QProcess> m_registry;

private Q_SLOTS:
    void initTestCase()
    {
        QDir endpointdir(QFileInfo(TEST_RUNTIME_CONFIG).dir());
        endpointdir.cd(QString("endpoints"));
        QFile::remove(SCOPES_TMP_ENDPOINT_DIR);
        // symlinking to workaround "File name too long" issue
        QVERIFY2(QFile::link(endpointdir.absolutePath(), SCOPES_TMP_ENDPOINT_DIR),
            "Unable to create symlink " SCOPES_TMP_ENDPOINT_DIR);
        // startup our private scope registry
        QString registryBin(TEST_SCOPEREGISTRY_BIN);
        QStringList arguments;
        arguments << TEST_RUNTIME_CONFIG;

        m_registry.reset(new QProcess(nullptr));
        m_registry->start(registryBin, arguments);
    }

    void cleanupTestCase()
    {
        if (m_registry) {
            m_registry->terminate();
            if (!m_registry->waitForFinished()) {
                m_registry->kill();
            }
        }
        QFile::remove(SCOPES_TMP_ENDPOINT_DIR);
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
        // wait for the search to finish
        QTRY_COMPARE(m_scope->searchInProgress(), false);

        // ensure categories have > 0 rows
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());

        // ensure results have some data
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);

        // test also the ResultsModel::get() method
        QVariantMap result_data(results->get(0).toMap());
        QVERIFY(result_data.size() > 0);
        QVERIFY(result_data.contains("uri"));
        QVERIFY(result_data.contains("categoryId"));
        QVERIFY(result_data.contains("result"));
        QVERIFY(result_data.contains("title"));
    }

    void testScopesGet()
    {
        QVariant scope_var = m_scopes->get(0);
        QVERIFY(scope_var.canConvert<scopes_ng::Scope*>());

        // try incorrect index as well
        scope_var = m_scopes->get(65536);
        QVERIFY(scope_var.isNull());
        scope_var = m_scopes->get(-1);
        QVERIFY(scope_var.isNull());
    }

    void testScopeProperties()
    {
        QCOMPARE(m_scope->id(), QString("mock-scope"));
        QCOMPARE(m_scope->name(), QString("mock.DisplayName"));
        QCOMPARE(m_scope->iconHint(), QString("mock.Icon"));
        QCOMPARE(m_scope->description(), QString("mock.Description"));
        QCOMPARE(m_scope->searchHint(), QString("mock.SearchHint"));
    }

    void testTwoSearches()
    {
        QCOMPARE(m_scope->searchInProgress(), false);
        // perform a search
        m_scope->setSearchQuery(QString(""));
        QCOMPARE(m_scope->searchInProgress(), true);
        // wait for the search to finish
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
        // wait for the search to finish
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

    void testResultMetadata()
    {
        QCOMPARE(m_scope->searchInProgress(), false);
        // perform a search
        m_scope->setSearchQuery(QString("metadata"));
        QCOMPARE(m_scope->searchInProgress(), true);
        // wait for the search to finish
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
        // mapped to the same field name
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleSubtitle).toString(), QString("subtitle"));
        // mapped to a different field name
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleEmblem).toString(), QString("emblem"));
        // mapped but not present in the result
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleMascot).toString(), QString());
        // unmapped
        QVERIFY(results->data(idx, ResultsModel::Roles::RoleAltRating).isNull());
        QVERIFY(results->data(idx, ResultsModel::Roles::RoleOldPrice).isNull());
        QVERIFY(results->data(idx, ResultsModel::Roles::RolePrice).isNull());
        QVERIFY(results->data(idx, ResultsModel::Roles::RoleAltPrice).isNull());
        QVERIFY(results->data(idx, ResultsModel::Roles::RoleRating).isNull());
        QVERIFY(results->data(idx, ResultsModel::Roles::RoleSummary).isNull());
    }

    void testAlbumArtResult()
    {
        QCOMPARE(m_scope->searchInProgress(), false);
        // perform a search
        m_scope->setSearchQuery(QString("music"));
        QCOMPARE(m_scope->searchInProgress(), true);
        // wait for the search to finish
        QTRY_COMPARE(m_scope->searchInProgress(), false);

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
        QCOMPARE(m_scope->searchInProgress(), false);
        // perform a search
        m_scope->setSearchQuery(QString("metadata"));
        QCOMPARE(m_scope->searchInProgress(), true);
        // wait for the search to finish
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

        // should have 1 category now
        QCOMPARE(categories->rowCount(), 1);
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
        QCOMPARE(m_scope->searchInProgress(), false);
        // perform a search
        m_scope->setSearchQuery(QString("rating"));
        QCOMPARE(m_scope->searchInProgress(), true);
        // wait for the search to finish
        QTRY_COMPARE(m_scope->searchInProgress(), false);

        // get ResultsModel instance
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);

        auto idx = results->index(0);
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleTitle).toString(), QString("result for: \"rating\""));
        QCOMPARE(results->data(idx, ResultsModel::Roles::RoleRating).toString(), QString("***"));
    }

    void testCategoryDefaults()
    {
        // this search return minimal category definition, defaults should kick in
        m_scope->setSearchQuery(QString("minimal"));
        QCOMPARE(m_scope->searchInProgress(), true);
        // wait for the search to finish
        QTRY_COMPARE(m_scope->searchInProgress(), false);

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
        QCOMPARE(renderer.value("card-size"), QJsonValue(QString("medium")));
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
        m_scope->setSearchQuery(QString(""));
        QCOMPARE(m_scope->searchInProgress(), true);
        // wait for the search to finish
        QTRY_COMPARE(m_scope->searchInProgress(), false);

        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);

        qRegisterMetaType<QVector<int>>();
        QSignalSpy spy(categories, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&, const QVector<int>&)));

        // should at least change components
        m_scope->setSearchQuery(QString("metadata"));
        QCOMPARE(m_scope->searchInProgress(), true);
        // wait for the search to finish
        QTRY_COMPARE(m_scope->searchInProgress(), false);

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

    void testScopePreview()
    {
        QCOMPARE(m_scope->searchInProgress(), false);
        // perform a search
        m_scope->setSearchQuery(QString(""));
        QCOMPARE(m_scope->searchInProgress(), true);
        // wait for the search to finish
        QTRY_COMPARE(m_scope->searchInProgress(), false);

        // ensure categories have > 0 rows
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());

        // ensure results have some data
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);
        auto result_var = results->data(results->index(0), ResultsModel::RoleResult);
        QCOMPARE(result_var.isNull(), false);
        auto result = result_var.value<std::shared_ptr<unity::scopes::Result>>();

        auto preview_stack = m_scope->preview(result_var);
        QCOMPARE(preview_stack->rowCount(), 1);
        QCOMPARE(preview_stack->widgetColumnCount(), 1);
        auto preview_var = preview_stack->data(preview_stack->index(0), PreviewStack::RolePreviewModel);
        auto preview_model = preview_stack->get(0);
        QCOMPARE(preview_model, preview_var.value<scopes_ng::PreviewModel*>());
        QCOMPARE(preview_model->widgetColumnCount(), 1);
        QTRY_COMPARE(preview_model->loaded(), true);

        auto preview_widgets = preview_model->data(preview_model->index(0), PreviewModel::RoleColumnModel).value<scopes_ng::PreviewWidgetModel*>();
        QCOMPARE(preview_widgets->rowCount(), 2);
        QVariantMap props;
        QModelIndex idx;

        idx = preview_widgets->index(0);
        QCOMPARE(preview_widgets->data(idx, PreviewWidgetModel::RoleWidgetId).toString(), QString("hdr"));
        QCOMPARE(preview_widgets->data(idx, PreviewWidgetModel::RoleType).toString(), QString("header"));
        props = preview_widgets->data(idx, PreviewWidgetModel::RoleProperties).toMap();
        QCOMPARE(props[QString("title")].toString(), QString::fromStdString(result->title()));
        QCOMPARE(props[QString("subtitle")].toString(), QString::fromStdString(result->uri()));
        QCOMPARE(props[QString("attribute-1")].toString(), QString("foo"));

        idx = preview_widgets->index(1);
        QCOMPARE(preview_widgets->data(idx, PreviewWidgetModel::RoleWidgetId).toString(), QString("img"));
        QCOMPARE(preview_widgets->data(idx, PreviewWidgetModel::RoleType).toString(), QString("image"));
        props = preview_widgets->data(idx, PreviewWidgetModel::RoleProperties).toMap();
        QVERIFY(props.contains("source"));
        QCOMPARE(props[QString("source")].toString(), QString::fromStdString(result->art()));
        QVERIFY(props.contains("zoomable"));
        QCOMPARE(props[QString("zoomable")].toBool(), false);
    }

    void testPreviewLayouts()
    {
        QCOMPARE(m_scope->searchInProgress(), false);
        // perform a search
        m_scope->setSearchQuery(QString("layout"));
        QCOMPARE(m_scope->searchInProgress(), true);
        // wait for the search to finish
        QTRY_COMPARE(m_scope->searchInProgress(), false);

        // ensure categories have > 0 rows
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());

        // ensure results have some data
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);
        auto result_var = results->data(results->index(0), ResultsModel::RoleResult);
        QCOMPARE(result_var.isNull(), false);
        auto result = result_var.value<std::shared_ptr<unity::scopes::Result>>();

        auto preview_stack = m_scope->preview(result_var);
        QCOMPARE(preview_stack->rowCount(), 1);
        QCOMPARE(preview_stack->widgetColumnCount(), 1);
        auto preview = preview_stack->get(0);
        QTRY_COMPARE(preview->loaded(), true);
        QCOMPARE(preview->rowCount(), 1);
        auto col_model1 = preview->data(preview->index(0), PreviewModel::RoleColumnModel).value<scopes_ng::PreviewWidgetModel*>();
        QCOMPARE(col_model1->rowCount(), 4);

        // switch the layout
        preview_stack->setWidgetColumnCount(2);
        QCOMPARE(preview->rowCount(), 2);
        QCOMPARE(col_model1->rowCount(), 1);
        auto col_model2 = preview->data(preview->index(1), PreviewModel::RoleColumnModel).value<scopes_ng::PreviewWidgetModel*>();
        QCOMPARE(col_model2->rowCount(), 3);

        // switch back
        preview_stack->setWidgetColumnCount(1);
        QCOMPARE(preview->rowCount(), 1);
        QCOMPARE(col_model1->rowCount(), 4);
    }

    void testScopeActivation()
    {
        QCOMPARE(m_scope->searchInProgress(), false);
        // perform a search
        m_scope->setSearchQuery(QString(""));
        QCOMPARE(m_scope->searchInProgress(), true);
        // wait for the search to finish
        QTRY_COMPARE(m_scope->searchInProgress(), false);

        // ensure categories have > 0 rows
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());

        // ensure results have some data
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);
        auto result_var = results->data(results->index(0), ResultsModel::RoleResult);
        QCOMPARE(result_var.isNull(), false);
        auto result = result_var.value<std::shared_ptr<unity::scopes::Result>>();

        QSignalSpy spy(m_scope, SIGNAL(hideDash()));
        m_scope->activate(result_var);
        QVERIFY(spy.wait());
    }

    void testPreviewAction()
    {
        QCOMPARE(m_scope->searchInProgress(), false);
        // perform a search
        m_scope->setSearchQuery(QString("layout"));
        QCOMPARE(m_scope->searchInProgress(), true);
        // wait for the search to finish
        QTRY_COMPARE(m_scope->searchInProgress(), false);

        // ensure categories have > 0 rows
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<ResultsModel*>());

        // ensure results have some data
        auto results = results_var.value<ResultsModel*>();
        QVERIFY(results->rowCount() > 0);
        auto result_var = results->data(results->index(0), ResultsModel::RoleResult);
        QCOMPARE(result_var.isNull(), false);
        auto result = result_var.value<std::shared_ptr<unity::scopes::Result>>();

        auto preview_stack = m_scope->preview(result_var);
        QCOMPARE(preview_stack->rowCount(), 1);
        QCOMPARE(preview_stack->widgetColumnCount(), 1);
        auto preview = preview_stack->get(0);
        QTRY_COMPARE(preview->loaded(), true);
        QCOMPARE(preview->rowCount(), 1);

        QSignalSpy spy(m_scope, SIGNAL(hideDash()));
        Q_EMIT preview->triggered(QString("actions"), QString("open"), QVariantMap());
        QVERIFY(spy.wait());
    }
};

QTEST_MAIN(ResultsTestNg)
#include <resultstest-ng.moc>
