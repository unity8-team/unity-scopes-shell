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

#include <scope-harness/matcher/category-matcher.h>
#include <scope-harness/matcher/category-list-matcher.h>
#include <scope-harness/matcher/preview-column-matcher.h>
#include <scope-harness/matcher/preview-matcher.h>
#include <scope-harness/matcher/preview-widget-matcher.h>
#include <scope-harness/matcher/result-matcher.h>
#include <scope-harness/results/category.h>
#include <scope-harness/results/result.h>
#include <scope-harness/scope-harness.h>

using namespace std;
namespace sh = unity::scopeharness;
namespace shm = unity::scopeharness::matcher;
namespace shr = unity::scopeharness::registry;
namespace shv = unity::scopeharness::view;
namespace sc = unity::scopes;
namespace ss = unity::shell::scopes;

class PreviewTest : public QObject
{
    Q_OBJECT
private:
    sh::ScopeHarness::UPtr m_harness;

    shv::ResultsView::SPtr m_resultsView;

private Q_SLOTS:
    void initTestCase()
    {
        m_harness = sh::ScopeHarness::newFromScopeList(
            shr::CustomRegistry::Parameters({
                TEST_DATA_DIR "mock-scope/mock-scope.ini"
            })
        );
    }

    void cleanupTestCase()
    {
        m_harness.reset();
    }

    void init()
    {
        m_resultsView = m_harness->resultsView();
        m_resultsView->setActiveScope("mock-scope");
    }

    void cleanup()
    {
        m_resultsView.reset();
    }

    void testScopePreview()
    {
        m_resultsView->setQuery("x");

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .hasAtLeast(1)
                .mode(shm::CategoryListMatcher::Mode::starts_with)
                .category(shm::CategoryMatcher("cat1")
                    .hasAtLeast(1)
                    .mode(shm::CategoryMatcher::Mode::starts_with)
                    .result(shm::ResultMatcher("test:uri"))
                )
                .match(m_resultsView->categories())
        );

        auto abstractView = m_resultsView->category(0).result(0).longPress();
        QVERIFY(bool(abstractView));
        auto previewView = dynamic_pointer_cast<shv::PreviewView>(abstractView);
        QVERIFY(bool(previewView));

        sc::VariantMap widget1
        {
            {"attribute-1", sc::Variant("foo")},
            {"subtitle", sc::Variant("test:uri")},
            {"title", sc::Variant("result for: \"x\"")}
        };
        sc::VariantMap widget2
        {
            {"source", sc::Variant("art")},
            {"zoomable", sc::Variant(false)}
        };

        QVERIFY_MATCHRESULT(
            shm::PreviewColumnMatcher()
            .column(
                shm::PreviewMatcher()
                .widget(
                    shm::PreviewWidgetMatcher("hdr")
                    .type("header")
                    .data(sc::Variant(widget1))
                )
                .widget(
                    shm::PreviewWidgetMatcher("img")
                    .type("image")
                    .data(sc::Variant(widget2))
                )
            )
            .match(previewView->widgets())
        );
    }

    void testExpandablePreviewWidget()
    {
//        QScopedPointer<PreviewStack> preview_stack;
//        QVERIFY(previewForFirstResult(m_scope, QString("expandable-widget"), preview_stack));
//        unity::scopes::Result::SPtr result;
//        QVERIFY(getFirstResult(m_scope->categories(), result));
//
//        QCOMPARE(preview_stack->rowCount(), 1);
//        QCOMPARE(preview_stack->widgetColumnCount(), 1);
//        auto preview_var = preview_stack->data(preview_stack->index(0), PreviewStack::RolePreviewModel);
//        auto preview_model = preview_stack->getPreviewModel(0);
//        QCOMPARE(preview_model, preview_var.value<scopes_ng::PreviewModel*>());
//        QCOMPARE(preview_model->widgetColumnCount(), 1);
//        QTRY_COMPARE(preview_model->loaded(), true);
//
//        auto preview_widgets = preview_model->data(preview_model->index(0), PreviewModel::RoleColumnModel).value<scopes_ng::PreviewWidgetModel*>();
//        QVERIFY(!preview_widgets->roleNames().isEmpty());
//        QCOMPARE(preview_widgets->rowCount(), 2);
//        QVariantMap props;
//        QModelIndex idx;
//
//        idx = preview_widgets->index(0);
//        QCOMPARE(preview_widgets->data(idx, PreviewWidgetModel::RoleWidgetId).toString(), QString("exp"));
//        QCOMPARE(preview_widgets->data(idx, PreviewWidgetModel::RoleType).toString(), QString("expandable"));
//        props = preview_widgets->data(idx, PreviewWidgetModel::RoleProperties).toMap();
//        QCOMPARE(props[QString("title")].toString(), QString("Expandable widget"));
//        auto widgets = props[QString("widgets")];
//        QVERIFY(widgets.canConvert<PreviewWidgetModel*>());
//
//        // test the model of widgets in expandable widget
//        {
//            auto widgets_model = widgets.value<PreviewWidgetModel*>();
//            idx = widgets_model->index(0);
//            QCOMPARE(widgets_model->rowCount(), 2);
//            QCOMPARE(widgets_model->data(idx, PreviewWidgetModel::RoleWidgetId).toString(), QString("txt"));
//            QCOMPARE(widgets_model->data(idx, PreviewWidgetModel::RoleType).toString(), QString("text"));
//            props = widgets_model->data(idx, PreviewWidgetModel::RoleProperties).toMap();
//            QCOMPARE(props[QString("title")].toString(), QString("Subwidget"));
//            QCOMPARE(props[QString("text")].toString(), QString("Lorum ipsum"));
//            idx = widgets_model->index(1);
//            QCOMPARE(widgets_model->data(idx, PreviewWidgetModel::RoleWidgetId).toString(), QString("img"));
//            QCOMPARE(widgets_model->data(idx, PreviewWidgetModel::RoleType).toString(), QString("image"));
//            props = widgets_model->data(idx, PreviewWidgetModel::RoleProperties).toMap();
//            QCOMPARE(props[QString("source")].toString(), QString("bar.png"));
//        }
//
//        idx = preview_widgets->index(1);
//        QCOMPARE(preview_widgets->data(idx, PreviewWidgetModel::RoleWidgetId).toString(), QString("img"));
//        QCOMPARE(preview_widgets->data(idx, PreviewWidgetModel::RoleType).toString(), QString("image"));
//        props = preview_widgets->data(idx, PreviewWidgetModel::RoleProperties).toMap();
//        QVERIFY(props.contains("source"));
//        QCOMPARE(props[QString("source")].toString(), QString("foo.png"));
    }

    void testPreviewLayouts()
    {
        m_resultsView->setQuery("layout");

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .hasAtLeast(1)
                .mode(shm::CategoryListMatcher::Mode::starts_with)
                .category(shm::CategoryMatcher("cat1")
                    .hasAtLeast(1)
                    .mode(shm::CategoryMatcher::Mode::starts_with)
                    .result(shm::ResultMatcher("test:layout"))
                )
                .match(m_resultsView->categories())
        );

        auto abstractView = m_resultsView->category(0).result(0).longPress();
        QVERIFY(bool(abstractView));
        auto previewView = dynamic_pointer_cast<shv::PreviewView>(abstractView);
        QVERIFY(bool(previewView));

        QVERIFY_MATCHRESULT(
            shm::PreviewColumnMatcher()
            .column(
                shm::PreviewMatcher()
                .widget(shm::PreviewWidgetMatcher("img"))
                .widget(shm::PreviewWidgetMatcher("hdr"))
                .widget(shm::PreviewWidgetMatcher("desc"))
                .widget(shm::PreviewWidgetMatcher("actions"))
            )
            .match(previewView->widgets())
        );

        previewView->setColumnCount(2);
        QVERIFY_MATCHRESULT(
            shm::PreviewColumnMatcher()
            .column(
                shm::PreviewMatcher()
                .widget(shm::PreviewWidgetMatcher("img"))
            )
            .column(
                shm::PreviewMatcher()
                .widget(shm::PreviewWidgetMatcher("hdr"))
                .widget(shm::PreviewWidgetMatcher("desc"))
                .widget(shm::PreviewWidgetMatcher("actions"))
            )
            .match(previewView->widgets())
        );

        previewView->setColumnCount(1);
        QVERIFY_MATCHRESULT(
            shm::PreviewColumnMatcher()
            .column(
                shm::PreviewMatcher()
                .widget(shm::PreviewWidgetMatcher("img"))
                .widget(shm::PreviewWidgetMatcher("hdr"))
                .widget(shm::PreviewWidgetMatcher("desc"))
                .widget(shm::PreviewWidgetMatcher("actions"))
            )
            .match(previewView->widgets())
        );
    }

    void testPreviewAction()
    {
        m_resultsView->setQuery("layout");

        auto abstractView = m_resultsView->category(0).result(0).longPress();
        QVERIFY(bool(abstractView));
        auto previewView = dynamic_pointer_cast<shv::PreviewView>(abstractView);
        QVERIFY(bool(previewView));

        QVERIFY_MATCHRESULT(
            shm::PreviewColumnMatcher()
            .column(
                shm::PreviewMatcher()
                .widget(shm::PreviewWidgetMatcher("img"))
                .widget(shm::PreviewWidgetMatcher("hdr"))
                .widget(shm::PreviewWidgetMatcher("desc"))
                .widget(shm::PreviewWidgetMatcher("actions"))
            )
            .match(previewView->widgets())
        );

        auto sameView = previewView->widgetsInFirstColumn().at("actions").trigger("hide", sc::Variant());
        QCOMPARE(abstractView, sameView);
        auto previewView2 = dynamic_pointer_cast<shv::PreviewView>(sameView);
        QVERIFY(bool(previewView2));
    }

    void testPreviewActionRequestingSearch()
    {
        m_resultsView->setQuery("query");

        auto abstractView = m_resultsView->category(0).result(0).longPress();
        QVERIFY(bool(abstractView));
        auto previewView = dynamic_pointer_cast<shv::PreviewView>(abstractView);
        QVERIFY(bool(previewView));

        QVERIFY_MATCHRESULT(
            shm::PreviewColumnMatcher()
            .column(
                shm::PreviewMatcher()
                .widget(shm::PreviewWidgetMatcher("actions"))
            )
            .match(previewView->widgets())
        );

        auto resView = previewView->widgetsInFirstColumn().at("actions").trigger("query",
                previewView->widgetsInFirstColumn().at("actions").data());
        // action with canned query uri should trigger a search
        auto resultsView2 = dynamic_pointer_cast<shv::ResultsView>(resView);
        QVERIFY(bool(resultsView2));
        QCOMPARE(m_resultsView, resultsView2);
    }

    void testPreviewReplacingPreview()
    {
        m_resultsView->setQuery("layout");

        auto abstractView = m_resultsView->category(0).result(0).longPress();
        QVERIFY(bool(abstractView));
        auto previewView = dynamic_pointer_cast<shv::PreviewView>(abstractView);
        QVERIFY(bool(previewView));

        QVERIFY_MATCHRESULT(
            shm::PreviewColumnMatcher()
            .column(
                shm::PreviewMatcher()
                .widget(shm::PreviewWidgetMatcher("img"))
                .widget(shm::PreviewWidgetMatcher("hdr"))
                .widget(shm::PreviewWidgetMatcher("desc"))
                .widget(shm::PreviewWidgetMatcher("actions"))
            )
            .match(previewView->widgets())
        );

        sc::VariantMap hints {{"session-id", sc::Variant("goo")}};
        auto sameView = previewView->widgetsInFirstColumn().at("actions").trigger("download", sc::Variant(hints));
        auto previewView2 = dynamic_pointer_cast<shv::PreviewView>(sameView);
        QVERIFY(bool(previewView2));

        QVERIFY_MATCHRESULT(
            shm::PreviewColumnMatcher()
            .column(
                shm::PreviewMatcher()
                .widget(shm::PreviewWidgetMatcher("img"))
                .widget(shm::PreviewWidgetMatcher("hdr"))
                .widget(shm::PreviewWidgetMatcher("desc"))
                .widget(shm::PreviewWidgetMatcher("actions"))
                .widget(shm::PreviewWidgetMatcher("extra"))
            )
            .match(previewView2->widgets())
        );
    }

};

QTEST_GUILESS_MAIN(PreviewTest)
#include <previewtest.moc>
