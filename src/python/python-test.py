#!/usr/bin/env python3
from pyscope_harness import *
from pyscope_harness.testing import *
import unittest

TEST_DATA_DIR='/home/vivid/python-harness/tests/data/'

class SimpleResultsTest (ScopeHarnessTestCase):
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

    def test_1(self):
        match = CategoryListMatcher().has_at_least(1).match(self.view.categories)
        self.assertMatchResult(match)

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

if __name__ == '__main__':
    unittest.main()
