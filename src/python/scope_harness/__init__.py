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

"""

# FIXME: it would be really nice to include the list of classes from _scope_harness in the scope of the above docstring,
# cause the classes *are* imported and visible in the current scope.

from ._scope_harness import *
__all__ = ['_scope_harness']
