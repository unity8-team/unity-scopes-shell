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

from unittest import TestCase
from pyscope_harness import MatchResult

class ScopeHarnessTestCase (TestCase):
    """ A class whose instances are single test cases.

    This class extends unittest.TestCase with helper methods relevant for testing of Unity scopes.
    """

    def assertMatchResult(self, match_result):
        """ Assert for MatchResult object that fails if match wasn't successful and prints
            conditions which were not met by the matcher.
        """
        self.assertIsInstance(match_result, MatchResult, msg='match_result must be an instance of MatchResult')
        self.assertTrue(match_result.success, msg=match_result.concat_failures)
