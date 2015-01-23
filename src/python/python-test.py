#!/usr/bin/env python3
from pyscope_harness import *
from pyscope_harness.testing import *
import unittest

TEST_DATA_DIR='/home/vivid/python-harness/tests/data/'

class ResultsTest (ScopeHarnessTestCase):
    @classmethod
    def setUpClass(cls):
        cls.harness = ScopeHarness.new_from_scope_list(Parameters([
            TEST_DATA_DIR + "mock-scope/mock-scope.ini",
            TEST_DATA_DIR + "mock-scope-info/mock-scope-info.ini",
            TEST_DATA_DIR + "mock-scope-ttl/mock-scope-ttl.ini"
            ]))
        cls.view = cls.harness.results_view
        cls.view.active_scope = "mock-scope"
        cls.view.search_query = ""

    def test_basic_result(self):
        match = CategoryListMatcher() \
            .has_at_least(1) \
            .mode(CategoryListMatcherMode.BY_ID) \
            .category(CategoryMatcher("cat1") \
                    .has_at_least(1) \
                    .mode(CategoryMatcherMode.BY_URI) \
                    .result(ResultMatcher("test:uri") \
                    .properties({'title': 'result for: ""', 'art':'art'}) \
                    .dnd_uri("test:dnd_uri") \
                    )) \
            .match(self.view.categories)
        self.assertMatchResult(match)

class PreviewTest (ScopeHarnessTestCase):
    @classmethod
    def setUpClass(cls):
        cls.harness = ScopeHarness.new_from_scope_list(Parameters([
            TEST_DATA_DIR + "mock-scope/mock-scope.ini",
            ]))
        cls.view = cls.harness.results_view
        cls.view.active_scope = "mock-scope"
        cls.view.search_query = ""


    def test_preview_layouts(self):
        self.view.search_query = "layout"
        match = CategoryListMatcher() \
                .has_at_least(1) \
                .mode(CategoryListMatcherMode.STARTS_WITH) \
                .category(CategoryMatcher("cat1") \
                        .has_at_least(1)
                        .mode(CategoryMatcherMode.STARTS_WITH) \
                        .result(ResultMatcher("test:layout")) \
                        ).match(self.view.categories)
        self.assertMatchResult(match)

        pview = self.view.category(0).result(0).activate()
        self.assertIsInstance(pview, PreviewView)

        match2 = PreviewColumnMatcher().column( \
                PreviewMatcher() \
                    .widget(PreviewWidgetMatcher("img")) \
                    .widget(PreviewWidgetMatcher("hdr")) \
                    .widget(PreviewWidgetMatcher("desc")) \
                    .widget(PreviewWidgetMatcher("actions")) \
                ).match(pview.widgets)

        pview.column_count = 2
        match3 = PreviewColumnMatcher() \
                 .column(PreviewMatcher() \
                         .widget(PreviewWidgetMatcher("img"))) \
                 .column(PreviewMatcher() \
                         .widget(PreviewWidgetMatcher("hdr")) \
                         .widget(PreviewWidgetMatcher("desc")) \
                         .widget(PreviewWidgetMatcher("actions")) \
                        ).match(pview.widgets)
        self.assertMatchResult(match3)

        pview.column_count = 1
        match4 = PreviewColumnMatcher() \
                 .column(PreviewMatcher() \
                         .widget(PreviewWidgetMatcher("img")) \
                         .widget(PreviewWidgetMatcher("hdr")) \
                         .widget(PreviewWidgetMatcher("desc")) \
                         .widget(PreviewWidgetMatcher("actions")) \
                        ).match(pview.widgets)
        self.assertMatchResult(match4)

    def test_preview_action(self):
        self.view.search_query = "layout"
        pview = self.view.category(0).result(0).activate()
        self.assertIsInstance(pview, PreviewView)
        match = PreviewColumnMatcher() \
                 .column(PreviewMatcher() \
                         .widget(PreviewWidgetMatcher("img")) \
                         .widget(PreviewWidgetMatcher("hdr")) \
                         .widget(PreviewWidgetMatcher("desc")) \
                         .widget(PreviewWidgetMatcher("actions")) \
                        ).match(pview.widgets)
        self.assertMatchResult(match)

        next_view = pview.widgets_in_first_column["actions"].trigger("hide", None)
        self.assertEqual(pview, next_view)

    def test_preview_replacing_preview(self):
        self.view.search_query = "layout"
        pview = self.view.category(0).result(0).activate()
        self.assertIsInstance(pview, PreviewView)
        match = PreviewColumnMatcher() \
                .column(PreviewMatcher() \
                        .widget(PreviewWidgetMatcher("img")) \
                        .widget(PreviewWidgetMatcher("hdr")) \
                        .widget(PreviewWidgetMatcher("desc")) \
                        .widget(PreviewWidgetMatcher("actions")) \
                       ).match(pview.widgets)
        self.assertMatchResult(match)

        hints = {"session-id": "goo"};
        pview2 = pview.widgets_in_first_column["actions"].trigger("download", hints)

        match2 = PreviewColumnMatcher() \
                .column(PreviewMatcher() \
                        .widget(PreviewWidgetMatcher("img")) \
                        .widget(PreviewWidgetMatcher("hdr")) \
                        .widget(PreviewWidgetMatcher("desc")) \
                        .widget(PreviewWidgetMatcher("actions")) \
                        .widget(PreviewWidgetMatcher("extra")) \
                       ).match(pview2.widgets)
        self.assertMatchResult(match2)

class DepartmentsTest (ScopeHarnessTestCase):
    @classmethod
    def setUpClass(cls):
        cls.harness = ScopeHarness.new_from_scope_list(Parameters([
                    TEST_DATA_DIR + "mock-scope-departments/mock-scope-departments.ini",
                    TEST_DATA_DIR + "mock-scope-double-nav/mock-scope-double-nav.ini",
                    TEST_DATA_DIR + "mock-scope-departments-flipflop/mock-scope-departments-flipflop.ini"
            ]))
        cls.view = cls.harness.results_view

    def test_no_departments(self):
        self.view.active_scope = 'mock-scope-departments'
        self.view.search_query = 'foo'

        self.assertFalse(self.view.has_navigation)
        self.assertFalse(self.view.has_alt_navigation)

    def test_root_department(self):
        self.view.active_scope = 'mock-scope-departments'
        self.view.search_query = ''

        self.assertTrue(self.view.has_navigation)
        self.assertFalse(self.view.has_alt_navigation)
        self.assertEqual(self.view.department_id, '')

        departments = self.view.browse_department('')
        self.assertEqual(departments.label, 'All departments')
        self.assertEqual(departments.all_label, '')
        self.assertEqual(departments.parent_id, '')
        self.assertTrue(departments.is_root)
        self.assertEqual(len(departments), 5)

        dep = departments[0]
        self.assertEqual(dep.id, 'books')
        self.assertEqual(dep.label, 'Books')
        self.assertTrue(dep.has_children)
        self.assertFalse(dep.is_active)

        # excercise different methods for getting children
        #dep2 = departments.child(0)
        #dep3 = departments.children[0]
        #self.assertEqual(dep, dep2)
        #self.assertEqual(dep2, dep3)

        dep = departments[4]
        self.assertEqual(dep.id, 'toys')
        self.assertEqual(dep.label, 'Toys, Children & Baby')
        self.assertTrue(dep.has_children)
        self.assertFalse(dep.is_active)

if __name__ == '__main__':
    unittest.main()
