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
        cls.view.set_active_scope("mock-scope")
        cls.view.search_query = "minimal"
        #cls.view.wait_for_results_change()

    def test_1(self):
        match = CategoryListMatcher().has_at_least(1).match(self.view.categories)
        self.assertMatchResult(match)

    def test_2(self):
        match = CategoryListMatcher() \
            .has_at_least(1) \
            .mode(CategoryListMatcherMode.BY_ID) \
            .category(CategoryMatcher("cat1") \
                    .has_at_least(1) \
                    .mode(CategoryMatcherMode.BY_URI) \
                    .result(ResultMatcher("test:uri") \
                    .properties({'title': 'result for: "minimal"'}) \
                    .art("") \
                    )) \
            .match(self.view.categories)
        self.assertMatchResult(match)

if __name__ == '__main__':
    unittest.main()
