from unittest import TestCase

class ScopeHarnessTestCase (TestCase):
    def assertMatchResult(match_result):
        self.assertIsInstance(match_result, pyscope_harness.MatchResult, msg='match_result must be an instance of MatchResult')
        self.assertTrue(match_result.success, msg=match_result.concat_failures)
