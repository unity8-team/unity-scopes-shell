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

""" Scope harness Python bindings.

It makes testing scopes easy with classes that help writing high-level
assertions about expected results, categories etc.

Here is a simple example for test case utilizing scope_harness. It is build upon
the standard unittest framework (by inheriting from ScopeHarnessTestCase, based on unittest.TestCase),
but there no obligation to use it - the only functionality that ScopeHarnessTestCase provides is a
helper assertMatchResult method, that can easily be replaced with a custom implementation.

from scope_harness import *
from scope_harness.testing import ScopeHarnessTestCase
import unittest

class MyScopeTest(ScopeHarnessTestCase):
    @classmethod
    def setUpClass(cls):
        cls.harness = ScopeHarness.new_from_scope_list(Parameters([
            "myscope/myscope.ini"
            ]))

    def setUp(self):
        self.view = self.harness.results_view
        self.view.active_scope = 'myscope'

    def test_surfacing_results(self):
        self.view.browse_department('')
        self.view.search_query = ''

        # Check first results of first two categories (out of 3 expected categories)
        self.assertMatchResult(CategoryListMatcher()
            .has_exactly(3)
            .mode(CategoryListMatcherMode.BY_ID)
            .category(CategoryMatcher("mycat1")
                    .has_at_least(1)
                    .mode(CategoryMatcherMode.BY_URI)
                    .result(ResultMatcher("http://myscopeuri1")
                    .title('Result 1')
            ))
            .category(CategoryMatcher("mycat2")
                      .has_at_least(1)
                      .mode(CategoryMatcherMode.STARTS_WITH)
                      .result(ResultMatcher("http://myscopeuri2")
                      .properties({'myboolattribute': True, 'mystringattribute': 'a'})
                      .title('Result 2')
            ))
            .match(self.view.categories))

"""

# FIXME: it would be really nice to include the list of classes from _scope_harness in the scope of the above docstring,
# cause the classes *are* imported and visible in the current scope.

from ._scope_harness import *
