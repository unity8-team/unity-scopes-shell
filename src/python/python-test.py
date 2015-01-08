#!/usr/bin/env python3
from harnesspy import *

TEST_DATA_DIR='/home/vivid/python-harness/tests/data/'

harness = ScopeHarness.new_from_scope_list(Parameters([
            TEST_DATA_DIR + "mock-scope/mock-scope.ini",
            TEST_DATA_DIR + "mock-scope-info/mock-scope-info.ini",
            TEST_DATA_DIR + "mock-scope-ttl/mock-scope-ttl.ini"
            ]))
view = harness.results_view
view.set_active_scope("mock-scope")
view.search_query = "minimal"
#view.wait_for_results_change()

print("TEST 1...")

matchres = CategoryListMatcher().has_at_least(1).match(view.categories)
print(matchres.success)
print(matchres.concat_failures)

print("TEST 2...")
match2 = CategoryListMatcher() \
    .has_at_least(1) \
    .mode(CategoryListMatcherMode.BY_ID) \
    .category(CategoryMatcher("cat1") \
            .has_at_least(1) \
            .mode(CategoryMatcherMode.BY_URI) \
            .result(ResultMatcher("test:uri") \
            .properties({'title': 'result for: "minimal"'}) \
            .art("") \
            )) \
    .match(view.categories)
print(match2.success)
print(match2.concat_failures)
