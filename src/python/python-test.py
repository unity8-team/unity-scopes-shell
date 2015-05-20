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

from scope_harness import ScopeHarness, CategoryMatcher, CategoryMatcherMode, CategoryListMatcher, SettingsMatcher, SettingsOptionMatcher
from scope_harness import SettingsMatcherMode, SettingsOptionType
from scope_harness import CategoryListMatcherMode, ResultMatcher, PreviewMatcher, PreviewWidgetMatcher, PreviewColumnMatcher, PreviewView
from scope_harness import Parameters, DepartmentMatcher, ChildDepartmentMatcher
from scope_harness.testing import ScopeHarnessTestCase
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

class SettingsTest(ScopeHarnessTestCase):
    @classmethod
    def setUpClass(cls):
        cls.harness = ScopeHarness.new_from_scope_list(Parameters([
                    TEST_DATA_DIR + "/mock-scope-departments/mock-scope-departments.ini",
                    TEST_DATA_DIR + "/mock-scope-double-nav/mock-scope-double-nav.ini",
                    TEST_DATA_DIR + "/mock-scope/mock-scope.ini"
            ]))
        cls.view = cls.harness.results_view

    def assertMatchResultFails(self, match_result, msg):
        self.assertFalse(match_result.success)
        self.assertEqual(match_result.concat_failures, msg)

    def test_basic(self):
        self.view.active_scope = 'mock-scope'
        settings = self.view.settings
        self.assertEqual(len(settings), 5)

        self.assertMatchResult(SettingsMatcher()
                .mode(SettingsMatcherMode.ALL)
                .has_at_least(1)
                .has_exactly(5)
                .option(SettingsOptionMatcher("location")
                    .display_name("Location")
                    .option_type(SettingsOptionType.STRING)
                    .value("London")
                    .default_value("London")
                    )
                .option(SettingsOptionMatcher("distanceUnit")
                    .display_name("Distance Unit")
                    .option_type(SettingsOptionType.LIST)
                    .display_values(["Kilometers", "Miles"])
                    .value("Miles")
                    .default_value("Miles")
                    )
                .option(SettingsOptionMatcher("age")
                    .display_name("Age")
                    .option_type(SettingsOptionType.NUMBER)
                    .value(23)
                    .default_value(23)
                    )
                .option(SettingsOptionMatcher("enabled")
                    .display_name("Enabled")
                    .option_type(SettingsOptionType.BOOLEAN)
                    .value(True)
                    .default_value(True)
                    )
                .option(SettingsOptionMatcher("color")
                    .display_name("Color")
                    .option_type(SettingsOptionType.STRING)
                    .value(None)
                    .default_value(None)
                    )
                .match(settings))

    def test_settings_change(self):
        self.view.active_scope = 'mock-scope'
        settings = self.view.settings

        settings.set("location", "Barcelona")

        self.assertMatchResult(
                SettingsMatcher()
                    .mode(SettingsMatcherMode.BY_ID)
                    .option(
                        SettingsOptionMatcher("location")
                            .value("Barcelona")
                            .default_value("London")
                        )
                    .match(settings)
                )

    def test_basic_failures(self):
        """
            Note: this test checks actual logic of scopes harness framework and shouldn't be replicated in
            tests of real scopes.
        """
        self.view.active_scope = 'mock-scope'
        settings = self.view.settings

        self.assertMatchResultFails(
                SettingsMatcher()
                    .has_at_least(6)
                    .match(settings),
                    "Failed expectations:\nExpected at least 6 options\n")

        self.assertMatchResultFails(
                SettingsMatcher()
                    .has_exactly(2)
                    .match(settings),
                    "Failed expectations:\nExpected exactly 2 options\n")

    def test_type_check_failures(self):
        """
            Note: this test checks actual logic of scopes harness framework and shouldn't be replicated in
            tests of real scopes.
        """
        self.view.active_scope = 'mock-scope'
        settings = self.view.settings

        self.assertMatchResultFails(
                SettingsMatcher()
                    .mode(SettingsMatcherMode.BY_ID)
                        .option(
                            SettingsOptionMatcher("age")
                                .option_type(SettingsOptionType.STRING)
                        )
                        .option(
                            SettingsOptionMatcher("distanceUnit")
                                .option_type(SettingsOptionType.BOOLEAN)
                        )
                        .option(
                            SettingsOptionMatcher("location")
                                .option_type(SettingsOptionType.NUMBER)
                        )
                        .option(
                            SettingsOptionMatcher("color")
                                .option_type(SettingsOptionType.LIST)
                        )
                        .match(settings),
                        "Failed expectations:\nOption ID age is of type number, expected string\nOption ID distanceUnit is of type list, expected boolean\nOption ID"
                        " location is of type string, expected number\nOption ID color is of type string, expected list\n")

    def test_value_check_failures(self):
        """
            Note: this test checks actual logic of scopes harness framework and shouldn't be replicated in
            tests of real scopes.
        """
        self.view.active_scope = 'mock-scope'
        settings = self.view.settings

        self.assertMatchResultFails(
                SettingsMatcher()
                    .mode(SettingsMatcherMode.BY_ID)
                    .option(
                        SettingsOptionMatcher("age")
                            .value("xyz")
                    )
                    .option(
                        SettingsOptionMatcher("distanceUnit")
                        .display_values(["Kilometers", "Parsecs"])
                    )
                    .match(settings),
                    "Failed expectations:\nOption with ID 'age' has 'value' == '23.0' but expected "
                    "'\"xyz\"'\nOption with ID 'distanceUnit' has 'displayValues' == '[\"Kilometers\",\"Miles\"]' but expected '[\"Kilometers\",\"Parsecs\"]'\n"
                )

    def test_value_set_failure(self):
        """
            Note: this test checks actual logic of scopes harness framework and shouldn't be replicated in
            tests of real scopes.
        """
        self.view.active_scope = 'mock-scope'
        settings = self.view.settings

        exception_thrown = False
        try:
            settings.set("distanceUnit", "foo")
        except ValueError as err:
            exception_thrown = True
            self.assertEqual(str(err), "Failed to update settings option with ID 'distanceUnit': no such value 'foo'")

        self.assertTrue(exception_thrown)

        exception_thrown = False
        try:
            settings.set("xyz", "abc")
        except ValueError as err:
            exception_thrown = True
            self.assertEqual(str(err), "Setting update failed. No such option: 'xyz'")
        self.assertTrue(exception_thrown)

if __name__ == '__main__':
    unittest.main(argv = sys.argv[:1])
