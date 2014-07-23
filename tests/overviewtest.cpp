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

#include <scopes.h>
#include <scope.h>
#include <categories.h>
#include <overviewresults.h>
#include <previewmodel.h>
#include <previewstack.h>
#include <previewwidgetmodel.h>

#include "registry-spawner.h"
#include "test-utils.h"

using namespace scopes_ng;

class OverviewTest : public QObject
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

        // get scope proxy
        m_scope = qobject_cast<scopes_ng::Scope*>(m_scopes->overviewScope());
        QVERIFY(m_scope != nullptr);
    }

    void cleanup()
    {
        m_scopes.reset();
        m_scope = nullptr;
    }

    void testScopeProperties()
    {
        QCOMPARE(m_scope->id(), QString("scopes"));
        QCOMPARE(m_scope->name(), QString("mock.scopes.DisplayName"));
        QCOMPARE(m_scope->iconHint(), QString("/mock.Icon"));
        QCOMPARE(m_scope->description(), QString("mock.scopes.Description"));
        QCOMPARE(m_scope->searchHint(), QString("mock.SearchHint"));
        QCOMPARE(m_scope->shortcut(), QString("mock.HotKey"));
        QCOMPARE(m_scope->searchQuery(), QString());

        QCOMPARE(m_scope->isActive(), false);
        m_scope->setActive(true);
        QCOMPARE(m_scope->isActive(), true);
    }

    void testSurfacingQuery()
    {
        performSearch(m_scope, QString(""));

        // ensure categories have > 0 rows
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QCOMPARE(categories->data(categories->index(0), Categories::Roles::RoleCategoryId), QVariant(QString("favorites")));
        QCOMPARE(categories->data(categories->index(1), Categories::Roles::RoleCategoryId), QVariant(QString("all")));

        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<OverviewResultsModel*>());
        OverviewResultsModel* results = results_var.value<OverviewResultsModel*>();
        QVERIFY(results->rowCount() > 0);
    }

    void testPreview()
    {
        performSearch(m_scope, QString(""));

        // get a result from the model
        auto categories = m_scope->categories();
        QVERIFY(categories->rowCount() > 0);
        QVariant results_var = categories->data(categories->index(0), Categories::Roles::RoleResults);
        QVERIFY(results_var.canConvert<OverviewResultsModel*>());
        OverviewResultsModel* results = results_var.value<OverviewResultsModel*>();
        auto result_var = results->data(results->index(0), ResultsModel::RoleResult);
        QCOMPARE(result_var.isNull(), false);
        auto result = result_var.value<std::shared_ptr<unity::scopes::Result>>();
        QVERIFY(result != nullptr);

        // and try to preview it
        QScopedPointer<PreviewStack> preview_stack(static_cast<PreviewStack*>(m_scope->preview(QVariant::fromValue(result))));
        QCOMPARE(preview_stack->rowCount(), 1);
        QCOMPARE(preview_stack->widgetColumnCount(), 1);
        auto preview_var = preview_stack->data(preview_stack->index(0), PreviewStack::RolePreviewModel);
        auto preview_model = preview_stack->getPreviewModel(0);
        QCOMPARE(preview_model, preview_var.value<scopes_ng::PreviewModel*>());
        QCOMPARE(preview_model->widgetColumnCount(), 1);
        QTRY_COMPARE(preview_model->loaded(), true);

        auto preview_widgets = preview_model->data(preview_model->index(0), PreviewModel::RoleColumnModel).value<scopes_ng::PreviewWidgetModel*>();
        QVERIFY(!preview_widgets->roleNames().isEmpty());
        QCOMPARE(preview_widgets->rowCount(), 2);
    }

};

QTEST_GUILESS_MAIN(OverviewTest)
#include <overviewtest.moc>
