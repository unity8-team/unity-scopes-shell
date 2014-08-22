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
#include <QThread>
#include <QScopedPointer>
#include <QSignalSpy>
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

class PreviewTest : public QObject
{
    Q_OBJECT
private:
    QScopedPointer<RegistrySpawner> m_registry;
    QScopedPointer<Scopes> m_scopes;
    Scope* m_scope;

private Q_SLOTS:
    void initTestCase()
    {
        m_registry.reset(new RegistrySpawner());
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
        // should have at least one scope now
        QVERIFY(m_scopes->rowCount() > 1);

        QVariant scope_var = m_scopes->data(m_scopes->index(0), Scopes::Roles::RoleScope);
        QVERIFY(scope_var.canConvert<Scope*>());

        // get scope proxy
        m_scope = m_scopes->getScopeById("mock-scope");
        QVERIFY(m_scope != nullptr);
        m_scope->setActive(true);
    }

    void cleanup()
    {
        m_scopes.reset();
        m_scope = nullptr;
    }

    void testScopePreview()
    {
        QScopedPointer<PreviewStack> preview_stack;
        QVERIFY(previewForFirstResult(m_scope, QString(""), preview_stack));
        unity::scopes::Result::SPtr result;
        QVERIFY(getFirstResult(m_scope, result));

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

    void testExpandablePreviewWidget()
    {
        QScopedPointer<PreviewStack> preview_stack;
        QVERIFY(previewForFirstResult(m_scope, QString("expandable-widget"), preview_stack));
        unity::scopes::Result::SPtr result;
        QVERIFY(getFirstResult(m_scope, result));

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
        QVariantMap props;
        QModelIndex idx;

        idx = preview_widgets->index(0);
        QCOMPARE(preview_widgets->data(idx, PreviewWidgetModel::RoleWidgetId).toString(), QString("exp"));
        QCOMPARE(preview_widgets->data(idx, PreviewWidgetModel::RoleType).toString(), QString("expandable"));
        props = preview_widgets->data(idx, PreviewWidgetModel::RoleProperties).toMap();
        QCOMPARE(props[QString("title")].toString(), QString("Expandable widget"));
        auto widgets = props[QString("widgets")];
        QVERIFY(widgets.canConvert<PreviewWidgetModel*>());

        // test the model of widgets in expandable widget
        {
            auto widgets_model = widgets.value<PreviewWidgetModel*>();
            idx = widgets_model->index(0);
            QCOMPARE(widgets_model->rowCount(), 2);
            QCOMPARE(widgets_model->data(idx, PreviewWidgetModel::RoleWidgetId).toString(), QString("txt"));
            QCOMPARE(widgets_model->data(idx, PreviewWidgetModel::RoleType).toString(), QString("text"));
            props = widgets_model->data(idx, PreviewWidgetModel::RoleProperties).toMap();
            QCOMPARE(props[QString("title")].toString(), QString("Subwidget"));
            QCOMPARE(props[QString("text")].toString(), QString("Lorum ipsum"));
            idx = widgets_model->index(1);
            QCOMPARE(widgets_model->data(idx, PreviewWidgetModel::RoleWidgetId).toString(), QString("img"));
            QCOMPARE(widgets_model->data(idx, PreviewWidgetModel::RoleType).toString(), QString("image"));
            props = widgets_model->data(idx, PreviewWidgetModel::RoleProperties).toMap();
            QCOMPARE(props[QString("source")].toString(), QString("bar.png"));
        }

        idx = preview_widgets->index(1);
        QCOMPARE(preview_widgets->data(idx, PreviewWidgetModel::RoleWidgetId).toString(), QString("img"));
        QCOMPARE(preview_widgets->data(idx, PreviewWidgetModel::RoleType).toString(), QString("image"));
        props = preview_widgets->data(idx, PreviewWidgetModel::RoleProperties).toMap();
        QVERIFY(props.contains("source"));
        QCOMPARE(props[QString("source")].toString(), QString("foo.png"));
    }

    void testPreviewLayouts()
    {
        QScopedPointer<PreviewStack> preview_stack;
        QVERIFY(previewForFirstResult(m_scope, QString("layout"), preview_stack));

        QCOMPARE(preview_stack->rowCount(), 1);
        QCOMPARE(preview_stack->widgetColumnCount(), 1);
        auto preview = preview_stack->getPreviewModel(0);
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

    void testPreviewAction()
    {
        QScopedPointer<PreviewStack> preview_stack;
        QVERIFY(previewForFirstResult(m_scope, QString("layout"), preview_stack));

        QCOMPARE(preview_stack->rowCount(), 1);
        QCOMPARE(preview_stack->widgetColumnCount(), 1);
        auto preview = preview_stack->getPreviewModel(0);
        QTRY_COMPARE(preview->loaded(), true);
        QCOMPARE(preview->rowCount(), 1);

        QSignalSpy spy(m_scope, SIGNAL(hideDash()));
        Q_EMIT preview->triggered(QString("actions"), QString("hide"), QVariantMap());
        QCOMPARE(preview->processingAction(), true);
        QVERIFY(spy.wait());
        QCOMPARE(preview->processingAction(), false);
    }

    void testPreviewReplacingPreview()
    {
        QScopedPointer<PreviewStack> preview_stack;
        QVERIFY(previewForFirstResult(m_scope, QString("layout"), preview_stack));

        QCOMPARE(preview_stack->rowCount(), 1);
        QCOMPARE(preview_stack->widgetColumnCount(), 1);
        auto preview = preview_stack->getPreviewModel(0);
        QTRY_COMPARE(preview->loaded(), true);
        QCOMPARE(preview->rowCount(), 1);
        auto preview_widgets = preview->data(preview->index(0), PreviewModel::RoleColumnModel).value<scopes_ng::PreviewWidgetModel*>();
        QCOMPARE(preview_widgets->rowCount(), 4);

        QSignalSpy spy(preview, SIGNAL(loadedChanged()));
        QVariantMap hints;
        hints["session-id"] = QString("qoo");
        Q_EMIT preview->triggered(QString("actions"), QString("download"), hints);
        QCOMPARE(preview->processingAction(), true);
        // wait for loaded to become false
        QVERIFY(spy.wait());
        QCOMPARE(preview->loaded(), false);
        // a bit of gray area, preview was just marked as "about-to-be-replaced", so it kinda is processing the action?
        QCOMPARE(preview->processingAction(), true);
        QTRY_COMPARE(preview->loaded(), true);
        QCOMPARE(preview->processingAction(), false);
        // refresh widget model
        preview_widgets = preview->data(preview->index(0), PreviewModel::RoleColumnModel).value<scopes_ng::PreviewWidgetModel*>();
        QCOMPARE(preview_widgets->rowCount(), 5);
    }

};

QTEST_GUILESS_MAIN(PreviewTest)
#include <previewtest.moc>
