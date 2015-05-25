Writing scope harness tests with Python
#######################################

What is scope-harness
=====================
Scope-harness is a high-level testing framework for scopes that offers high-level abstractions to interact with scopes and simulate user interactions in order
to verify data (categories, results, departments etc.) returned by the scope. It can be used to implement tests executed as a part of the build process of a
scope.

Scope-harness is available via C++ API and also offers bindings for Python 3. Both C++ and Python APIs offer same functionality. This
documentation covers Python API only.

About the Python testing framework used
=======================================
Scope harness for Python is build upon the standard unittest framework (by inheriting from :class:`~scope_harness._scope_harness.ScopeHarnessTestCase`, based on unittest.TestCase),
but there no obligation to use it - the only functionality that ScopeHarnessTestCase provides is a
helper assertMatchResult method, that can easily be replaced with a custom implementation.

Here is the implementation of assertMatchResult for your reference.

.. code-block:: python

    from unittest import TestCase
    from scope_harness import MatchResult

    class ScopeHarnessTestCase(TestCase):
        """ A class whose instances are single test cases.

        This class extends unittest.TestCase with helper methods relevant for testing of Unity scopes.
        """

        def assertMatchResult(self, match_result):
            """ Assert for MatchResult object that fails if match wasn't successful and prints
                conditions which were not met by the matcher.
            """
            self.assertIsInstance(match_result, MatchResult, msg='match_result must be an instance of MatchResult')
            self.assertTrue(match_result.success, msg=match_result.concat_failures)

Getting started
===============
The main "entry point" for every scope harness test cases is an instance of ScopeHarness object. This object encapsulates various aspects of configuration of
scopes runtime, including an instance of scoperegistry - the central process which maintains the list of known scopes, separate from the scoperegistry instance and
scopes normally installed on your system.

When creating this object via one of its factory methods, you have to decide whether you want to run your tests against scoperegistry and scopes already installed on the system
(see :meth:`~scope_harness._scope_harness.ScopeHarness.new_from_system`), scopesregistry executed against an existing configuration file
(see :meth:`~scope_harness._scope_harness.ScopeHarness.new_from_pre_existing_config`) or a custom
scope registry instance which only knows about scopes provided by your test (:meth:ScopeHarness.ScopeHarness.new_from_scope_list). The latter is the most common use case. Consider
the following example of test setUpClass method which assumes two "dummy" scopes have been installed into your test directory, and TEST_DATA_DIR points to it.

.. code-block:: python

    from scope_harness import *
    from scope_harness.testing import ScopeHarnessTestCase
    import unittest

    class MyTest(ScopeHarnessTestCase):
        @classmethod
        def setUpClass(cls):
            cls.harness = ScopeHarness.new_from_scope_list(Parameters([
                TEST_DATA_DIR + "/myscope1/myscope1.ini",
                TEST_DATA_DIR + "/myscope2/myscope2.ini"
                ]))

Once ScopeHarness instance has been created, it provides the results_view property (a :any:ResultsView instance) which corresponds to a scope page in the unity8
dash; you can set curently active scope, its current search query, change active department and inspect the returned categories and their results.

Consider the following simple test:

.. code-block:: python

    class MyTest(ScopeHarnessTestCase):
        @classmethod
        def setUpClass(cls):
            cls.harness = ScopeHarness.new_from_scope_list(Parameters([
                TEST_DATA_DIR + "/myscope1/myscope1.ini"
                ]))
            cls.view = cls.harness.results_view

        def test_basic_result(self):
            self.view.active_scope = 'myscope1'
            self.view.search_query = ''
            self.assertMatchResult(CategoryListMatcher()
                .has_at_least(1)
                .mode(CategoryListMatcherMode.BY_ID)
                .category(
                        CategoryMatcher('mycategory1')
                            .has_at_least(1)
                            .mode(CategoryMatcherMode.BY_URI)
                            .result(ResultMatcher("myuri")
                                .properties({'title': 'mytitle', 'art':'myart'})
                                .dnd_uri("test:dnd_uri")
                        )
                ).match(self.view.categories)
        )

Note the following key features of scope harness shown in this test case:
    * there is no explicit "waiting" needed for state changes while asynchronous calls (such as setting a new search query) are dispatched; this is all built-in
      in the scope harness and abstracted away from the developer. This makes tests more robust and eliminates the "noise", making test code easier to read.
    * while ResultsView and other objects representing scope view and scope state have getters that can be used to examine and test for expected values, the
      recommended way of implementing the checks is via the family of "matcher" objects, such as CategoryListMatcher, CategoryMatcher and ResultMatcher. These matchers
      provide readable and concise way of expressing test scenarios, which resemble natural language and are more easy to understand than just a series of
      usual test case assertions.
    * also, the "match" methods of matchers produce a MatchResult instance object which provides a cumulative overview of all
      encountered errors, along with clear descriptions about the failing assertion, which is very convinient when used in conjunction with assertMatchResult
      helper method.
