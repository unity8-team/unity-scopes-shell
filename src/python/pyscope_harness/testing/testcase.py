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
