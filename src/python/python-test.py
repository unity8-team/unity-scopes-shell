#!/usr/bin/env python3
#
# Copyright (C) 2015 Canonical Ltd.
# Author: Pawel Stolowski <pawel.stolowski@canonical.com>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 3.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

"""
This is a test of scope harness python bindings against mock scopes.
The test cases here replicate some of the cases of the original C++ tests.
"""

from pyscope_harness import ScopeHarness, CategoryMatcher, CategoryMatcherMode, CategoryListMatcher
from pyscope_harness import CategoryListMatcherMode, ResultMatcher, PreviewMatcher, PreviewWidgetMatcher, PreviewColumnMatcher, PreviewView
from pyscope_harness import Parameters, DepartmentMatcher, ChildDepartmentMatcher
from pyscope_harness.testing import ScopeHarnessTestCase
import unittest
import sys

# first argument is the directory of test scopes
TEST_DATA_DIR = sys.argv[1]

class ResultsTest(ScopeHarnessTestCase):
    @classmethod
    def setUpClass(cls):
        cls.harness = ScopeHarness.new_from_scope_list(Parameters([
            TEST_DATA_DIR + "/mock-scope/mock-scope.ini",
            TEST_DATA_DIR + "/mock-scope-info/mock-scope-info.ini",
            TEST_DATA_DIR + "/mock-scope-ttl/mock-scope-ttl.ini"
            ]))
        cls.view = cls.harness.results_view
        cls.view.active_scope = "mock-scope"
        cls.view.search_query = ""

    def test_basic_result(self):
        self.assertMatchResult(CategoryListMatcher()
            .has_at_least(1)
            .mode(CategoryListMatcherMode.BY_ID)
            .category(CategoryMatcher("cat1")
                    .has_at_least(1)
                    .mode(CategoryMatcherMode.BY_URI)
                    .result(ResultMatcher("test:uri")
                    .properties({'title': 'result for: ""', 'art':'art'})
                    .dnd_uri("test:dnd_uri")
                    ))
            .match(self.view.categories))

class PreviewTest(ScopeHarnessTestCase):
    @classmethod
    def setUpClass(cls):
        cls.harness = ScopeHarness.new_from_scope_list(Parameters([
            TEST_DATA_DIR + "/mock-scope/mock-scope.ini",
            ]))
        cls.view = cls.harness.results_view
        cls.view.active_scope = "mock-scope"
        cls.view.search_query = ""


    def test_preview_layouts(self):
        self.view.search_query = "layout"
        self.assertMatchResult(CategoryListMatcher()
                .has_at_least(1)
                .mode(CategoryListMatcherMode.STARTS_WITH)
                .category(CategoryMatcher("cat1")
                        .has_at_least(1)
                        .mode(CategoryMatcherMode.STARTS_WITH)
                        .result(ResultMatcher("test:layout"))
                        ).match(self.view.categories))

        pview = self.view.category(0).result(0).tap()
        self.assertIsInstance(pview, PreviewView)

        self.assertMatchResult(PreviewColumnMatcher().column(
                PreviewMatcher()
                    .widget(PreviewWidgetMatcher("img"))
                    .widget(PreviewWidgetMatcher("hdr"))
                    .widget(PreviewWidgetMatcher("desc"))
                    .widget(PreviewWidgetMatcher("actions"))
                ).match(pview.widgets))

        pview.column_count = 2
        self.assertMatchResult(PreviewColumnMatcher()
                 .column(PreviewMatcher()
                         .widget(PreviewWidgetMatcher("img")))
                 .column(PreviewMatcher()
                         .widget(PreviewWidgetMatcher("hdr"))
                         .widget(PreviewWidgetMatcher("desc"))
                         .widget(PreviewWidgetMatcher("actions"))
                        ).match(pview.widgets))

        pview.column_count = 1
        self.assertMatchResult(PreviewColumnMatcher()
                 .column(PreviewMatcher()
                         .widget(PreviewWidgetMatcher("img"))
                         .widget(PreviewWidgetMatcher("hdr"))
                         .widget(PreviewWidgetMatcher("desc"))
                         .widget(PreviewWidgetMatcher("actions"))
                        ).match(pview.widgets))

    def test_preview_action(self):
        self.view.search_query = "layout"
        pview = self.view.category(0).result(0).tap()
        self.assertIsInstance(pview, PreviewView)
        self.assertMatchResult(PreviewColumnMatcher()
                 .column(PreviewMatcher()
                         .widget(PreviewWidgetMatcher("img"))
                         .widget(PreviewWidgetMatcher("hdr"))
                         .widget(PreviewWidgetMatcher("desc"))
                         .widget(PreviewWidgetMatcher("actions"))
                        ).match(pview.widgets))

        next_view = pview.widgets_in_first_column["actions"].trigger("hide", None)
        self.assertEqual(pview, next_view)

    def test_preview_replacing_preview(self):
        self.view.search_query = "layout"
        pview = self.view.category(0).result(0).tap()
        self.assertIsInstance(pview, PreviewView)
        self.assertMatchResult(PreviewColumnMatcher()
                .column(PreviewMatcher()
                        .widget(PreviewWidgetMatcher("img"))
                        .widget(PreviewWidgetMatcher("hdr"))
                        .widget(PreviewWidgetMatcher("desc"))
                        .widget(PreviewWidgetMatcher("actions"))
                       ).match(pview.widgets))

        hints = {"session-id": "goo"};
        pview2 = pview.widgets_in_first_column["actions"].trigger("download", hints)

        self.assertMatchResult(PreviewColumnMatcher()
                .column(PreviewMatcher()
                        .widget(PreviewWidgetMatcher("img"))
                        .widget(PreviewWidgetMatcher("hdr"))
                        .widget(PreviewWidgetMatcher("desc"))
                        .widget(PreviewWidgetMatcher("actions"))
                        .widget(PreviewWidgetMatcher("extra"))
                       ).match(pview2.widgets))

class DepartmentsTest(ScopeHarnessTestCase):
    @classmethod
    def setUpClass(cls):
        cls.harness = ScopeHarness.new_from_scope_list(Parameters([
                    TEST_DATA_DIR + "/mock-scope-departments/mock-scope-departments.ini",
                    TEST_DATA_DIR + "/mock-scope-double-nav/mock-scope-double-nav.ini",
                    TEST_DATA_DIR + "/mock-scope-departments-flipflop/mock-scope-departments-flipflop.ini"
            ]))
        cls.view = cls.harness.results_view

    def test_no_departments(self):
        self.view.active_scope = 'mock-scope-departments'
        self.view.search_query = 'foo'

        self.assertFalse(self.view.has_departments)
        self.assertFalse(self.view.has_alt_departments)

    def test_root_department(self):
        self.view.active_scope = 'mock-scope-departments'
        self.view.search_query = ''

        self.assertTrue(self.view.has_departments)
        self.assertFalse(self.view.has_alt_departments)
        self.assertEqual(self.view.department_id, '')

        departments = self.view.browse_department('')

        self.assertEqual(len(departments), 5)
        # excercise different methods for getting children
        dep = departments[0]
        dep2 = departments.child(0)
        dep3 = departments.children[0]
        self.assertEqual(dep.id, dep2.id)
        self.assertEqual(dep2.id, dep3.id)

        self.assertMatchResult(DepartmentMatcher()
            .has_exactly(5)
            .label('All departments')
            .all_label('')
            .parent_id('')
            .parent_label('')
            .is_root(True)
            .is_hidden(False)
            .child(ChildDepartmentMatcher('books')
                   .label('Books')
                   .has_children(True)
                   .is_active(False)
                   )
            .child(ChildDepartmentMatcher('movies'))
            .child(ChildDepartmentMatcher('electronics'))
            .child(ChildDepartmentMatcher('home'))
            .child(ChildDepartmentMatcher('toys')
                   .label('Toys, Children & Baby')
                   .has_children(True)
                   .is_active(False)
                   )
            .match(departments))

    def test_child_department(self):
        self.view.active_scope = 'mock-scope-departments'
        departments = self.view.browse_department('books')

if __name__ == '__main__':
    unittest.main(argv = sys.argv[:1])
