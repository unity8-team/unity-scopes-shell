#!/usr/bin/env python3
import harnesspy
#print(harnesspy.variant_demo())
harnesspy.run_qt()

TEST_DATA_DIR='/home/vivid/python-harness/tests/data/'

harness = harnesspy.ScopeHarness.new_from_scope_list(harnesspy.Parameters([
            TEST_DATA_DIR + "mock-scope/mock-scope.ini",
            TEST_DATA_DIR + "mock-scope-info/mock-scope-info.ini",
            TEST_DATA_DIR + "mock-scope-ttl/mock-scope-ttl.ini"
            ]))
view = harness.results_view
view.set_active_scope("mock-scope-ttl")
view.search_query = "foo"
view.wait_for_results_change()
print(len(view.categories))
for c in view.categories:
    print(c.id)
    print(c.title)

for r in view.categories[0].results:
    print(r.uri)
