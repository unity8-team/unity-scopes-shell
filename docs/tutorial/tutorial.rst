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
Scope harness for Python is build upon the standard unittest framework (by inheriting from :class:`~scope_harness.testing.ScopeHarnessTestCase`, based on unittest.TestCase),
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
The main "entry point" for every scope harness test cases is an instance of :class:`~ScopeHarness` object. This object encapsulates various aspects of configuration of
scopes runtime, including an instance of scoperegistry - the central process which maintains the list of known scopes, separate from the scoperegistry instance and
scopes normally installed on your system.

When creating this object via one of its factory methods, you have to decide whether you want to run your tests against scoperegistry and scopes already installed on the system
(see :meth:`~scope_harness.ScopeHarness.new_from_system`), scopesregistry executed against an existing configuration file
(see :meth:`~scope_harness.ScopeHarness.new_from_pre_existing_config`) or a custom
scope registry instance which only knows about scopes provided by your test (:meth:`~scope_harness.ScopeHarness.new_from_scope_list`). The latter is the most common use case.

Consider the following example of test setUpClass method which assumes two "dummy" scopes have been installed into your test directory, and TEST_DATA_DIR points
to it.

.. code-block:: python

    from scope_harness import *
    from scope_harness.testing import ScopeHarnessTestCase
    import unittest

    class MyTest(ScopeHarnessTestCase):
        @classmethod
        def setUpClass(cls):
            cls.harness = ScopeHarness.new_from_scope_list(
                    Parameters([
                        TEST_DATA_DIR + "/myscope1/myscope1.ini",
                        TEST_DATA_DIR + "/myscope2/myscope2.ini"
                    ])
            )

Once ScopeHarness instance has been created, it provides the results_view property (a :class:`~scope_harness.ResultsView` instance) which corresponds to a scope page in the unity8
dash; you can set curently active scope, its current search query, change active department, inspect the returned categories and their results etc.

Consider the following simple test:

.. code-block:: python
    :linenos:

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
            self.assertMatchResult(
                    CategoryListMatcher()
                        .has_at_least(2)
                        .mode(CategoryListMatcherMode.BY_ID)
                        .category(
                            CategoryMatcher('mycategory1')
                                .has_at_least(5)
                                .mode(CategoryMatcherMode.BY_URI)
                                    .result(
                                        ResultMatcher("myuri")
                                        .properties({'title': 'mytitle', 'art':'myart'})
                                        .dnd_uri("test:dnd_uri")
                                    )
                            ).match(self.view.categories)
                )

Here is line-by-line explanation of the checks performed by test_basic_failures test case:
    * 4-6 - create main :class:`~scope_harness.ScopeHarness` scope harness object to interact with scope(s).
    * 7 - store a reference to :class:`~scope_harness.ResultsView` object in the test case instance to reduce typing later.
    * 10 - Make 'myscope1' the active scope.
    * 11 - set search query value (executes a background search query).
    * 12-25 - verify the returned result(s) match expectations:
        * check that there are at least 2 categories in the view (lines 13-14);
        * pick a specific category by its ID (15-17) and check that it has at least 5 results (line 18);
        * enable picking results by uri in the :class:`~scope_harness.CategoryMatcher` (line 19) and verify there is a result with uri of "myuri" and given "title", "art" and
                                                        "dnd_uri" properties (lines 20-23).

Note the following key features of scope harness shown in the above test case:
    * there is no explicit "waiting" needed for state changes while asynchronous calls (such as setting a new search query) are dispatched; this is all built-in
      in the scope harness and abstracted away from the developer. This makes tests more robust and eliminates the "noise", making test code easier to read.
    * while ResultsView and other objects representing scope view and scope state have getters that can be used to examine and test for expected values, the
      recommended way of implementing the checks is via the family of "matcher" objects, such as CategoryListMatcher, CategoryMatcher and ResultMatcher. These matchers
      provide readable and concise way of expressing test scenarios, which resemble natural language and are more easy to understand than just a series of
      usual test case assertions.
    * also, the "match" methods of matchers produce a MatchResult instance object which provides a cumulative overview of all
      encountered errors, along with clear descriptions about the failing assertion, which is very convinient when used in conjunction with assertMatchResult
      helper method.

More on category and results matching modes
===========================================
When testing whether the list of categories returned by your scope matches expectations, you may verify the following characteristics of the list of categories via :class:`~scope_harness.CategoryListMatcher` and :class:`~scope_harness.CategoryMatcher`:
    * whether the list contains at least N categories, or exactly N categories: use :meth:`~scope_harness.CategoryListMatcher.has_at_least` or :meth:`~scope_harness.CategoryListMatcher.has_exactly`, respectively.
    * whether the list contains specific categories (some or all of them, and in the expected order):
        * to only verify if the list of categories contains specific categories (regardless of their position on the list),
          set the matching :meth:`~scope_harness.CategoryListMatcher.mode` to ``CategoryListMatcherMode.BY_ID`` and then pass expected categories via :class:`~scope_harness.CategoryMatcher` objects to :meth:`~scope_harness.CategoryListMatcher.category`.
        * to verify if the list starts with specific categories in the expected order (but possibly has more categories which you don't care about),
          set the matching :meth:`~scope_harness.CategoryListMatcher.mode` to ``CategoryListMatcherMode.STARTS_WITH`` and then pass expected categories as explained above.
        * to verify if the list contains all the expected categories and in the specific order set the matching :meth:`~scope_harness.CategoryListMatcherMode.mode` to ``CategoryListMatcherMode.ALL`` and then pass expected categories as explained above. In fact ``CategoryListMatcherMode.ALL`` is the default mode if you define any categories via :class:`~scope_harness.CategoryMatcher`, so setting the mode may as well by skipped.


When testing results withing a categories specified via :class:`~scope_harness.CategoryMatcher`, the following checks can be made:
    * whether the category has at least N results: use :meth:`~scope_harness.CategoryMatcher.has_at_least`.
    * whether the category contains specific results (some or all of them, in the specific order or disregarding the order):
        * to verify if the category contains specific results regardless of their position, set the matching :meth:`~scope_harness.CategoryMatcherMode` to
          ``CategoryMatcherMode.BY_URI`` and pass expected results via :class:`~scope_harness.ResultMatcher` objects to :meth:`~scope_harness.CategoryMatcher.result`.
        * to verify if the specific results appear first in the category, but the category possibly has more results which you don't care about, set the matching :meth:`~scope_harness.CategoryMatcherMode` to
          ``CategoryMatcherMode.STARTS_WITH`` and pass expected results as explained earlier.
        * to verify if the category contains all the expected results in the given order, set the matching :meth:`~scope_harness.CategoryMatcherMode` to
          ``CategoryMatcherMode.ALL`` and pass all results as explained above. This is the default matching mode if any :class:`~scope_harness.ResultMatcher` matchers are set for a category, so setting the mode can be omitted.


Here is an example of test case which checks if there are at least five categories returned, and then checks four of them by ID (the order of the categories is not verified). For the four expected categories the test verifies that they have at least one result each, and for the categories ``top-apps`` and ``our-favorite-games`` specific results are tested:
        * the ``top-apps`` category needs to have a at least one result, and the first result of that category is matched against the provided :class:`~scope_harness.ResultMatcher`.
        * the ``our-favorite-games`` category needs to have at least one result, and the result specified by the the provided :class:`~scope_harness.ResultMatcher`
          needs to appear somewhere in that category, but it doesn't need to be the first one thanks to ``CategoryMatcherMode.BY_URI``.

.. code-block:: python

    def test_results(self):
        self.view.search_query = ''

        self.assertMatchResult(
            CategoryListMatcher()
                .has_at_least(5)
                .mode(CategoryListMatcherMode.BY_ID)
                .category(CategoryMatcher("app-of-the-week")
                        .has_at_least(1)
                        )
                .category(CategoryMatcher("top-apps")
                        .has_at_least(1)
                        .mode(CategoryMatcherMode.STARTS_WITH)
                        .result(ResultMatcher("https://search.apps.ubuntu.com/api/v1/package/com.ubuntu.developer.bobo1993324.udropcabin")
                        .title('uDropCabin')
                        .subtitle('Zhang Boren')
                ))
                .category(CategoryMatcher("our-favorite-games")
                        .has_at_least(1)
                        .mode(CategoryMatcherMode.BY_URI)
                        .result(ResultMatcher("https://search.apps.ubuntu.com/api/v1/package/com.ubuntu.developer.andrew-hayzen.volleyball2d") \
                ))
                .category(CategoryMatcher("travel-apps")
                        .has_at_least(1))
                .match(self.view.categories))

Testing departments
===================

Departments can be "browsed" by calling :meth:`~scope_harness.ResultsView.browse_department` method; changing the department invokes a new search and the method
returns the new list of departments. The list of departments can be tested using :class:`~scope_harness.DepartmentMatcher` and :class:`~scope_harness.ChildDepartmentMatcher` matchers.
The ``DepartmentMatcher`` support three modes of matching (``DepartmentMatcherMode.ALL``, ``DepartmentMatcherMode.STARTS_WITH`` and ``DepartmentMatcherMode.BY_ID``) which have the same semantics as with ``CategoryMatcher`` or ``CategoryListMatcher`` described above.

Here is an example of two departments tests: the first test case checks the starting list of departments (the surfacing mode), the second case simulates browsing of ``games`` sub-department, verifies it has no further sub-departments and also verifies the returned categories.

Note: the empty department ID corresponds to the root department.

.. code-block:: python

    def test_surfacing_departments(self):
        self.view.search_query = ''
        departments = self.view.browse_department('')
        self.assertMatchResult(
                DepartmentMatcher()
                    .mode(DepartmentMatcherMode.STARTS_WITH)
                        .id('')
                        .label('All')
                        .all_label('')
                        .parent_id('')
                        .parent_label('')
                        .is_root(True)
                        .is_hidden(False)
                        .child(ChildDepartmentMatcher('business'))
                        .child(ChildDepartmentMatcher('communication'))
                        .child(ChildDepartmentMatcher('education'))
                        .child(ChildDepartmentMatcher('entertainment'))
                        .child(ChildDepartmentMatcher('finance'))
                        .child(ChildDepartmentMatcher('games'))
                        .child(ChildDepartmentMatcher('graphics'))
                        .child(ChildDepartmentMatcher('accessories'))
                        .child(ChildDepartmentMatcher('weather'))
                        .match(departments))

    def test_department_browsing(self):
        self.view.search_query = ''
        departments = self.view.browse_department('games')
        self.assertMatchResult(DepartmentMatcher()
            .has_exactly(0)
            .mode(DepartmentMatcherMode.STARTS_WITH)
            .label('Games')
            .all_label('')
            .parent_id('')
            .parent_label('All')
            .is_root(False)
            .is_hidden(False)
            .match(departments))

        self.assertMatchResult(CategoryListMatcher()
            .has_exactly(3)
            .mode(CategoryListMatcherMode.BY_ID)
            .category(CategoryMatcher("top-games")
                      .has_at_least(1)
            )
            .category(CategoryMatcher("all-scopes")
                      .has_at_least(1)
            )
            .category(CategoryMatcher("all-apps")
                      .has_at_least(1)
            )
            .match(self.view.categories))

Testing previews
================

Previews can be invoked by calling :meth:`~scope_harness.Result.tap` method of the result. Note that ``tapping`` the result will - in cases where result's
``uri`` is a canned scope query (i.e. ``scope://`` uri) - execute a new search and return a :class:`~scope_harness.ResultsView` instance; in other cases a
:class:`~scope_harness.PreviewView` will be returned. This conditions are verified by checks in lines 5 and 37.

Below is an example of test cases covering preview widgets. The ``test_preview_layouts`` test case verifies different column layouts within the preview.
The second test case simulates activation of preview action by calling :meth:`~scope_harness.PreviewWidget.trigger` (line 47) and verifies the same preview is
returned in response.

.. code-block:: python
    :linenos:

    def test_preview_layouts(self):
        self.view.search_query = ''

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
        self.view.search_query = ''
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

Using scope settings
====================

Settings exported by scopes can be accessed via :meth:`~scope_harness.ResultsView.settings` property and tested using :class:`~scope_harness.SettingsMatcher`.
The :class:`~scope_harness.SettingsView` object returned by the above method :meth:`~scope_harness.SettingsView.set` method that can be used to modify settings
(simulate user choices). Note that ``set`` method is loosely-typed (the new value is an object / variant), that means the correct data type needs to be passed
to it, depending on the type of setting to modify:
             * for a setting of ``number`` type, pass an integer or float number.
             * for a setting of ``string`` type, pass a string value.
             * for a setting of ``list`` type, pass the string value corresponding to one of the supported choices.
             * for a setting of ``boolean`` type, pass True / False literals.

Changing a setting value refreshes search results.

Here is an example of a test case which modifies a setting value (this test should of course also check the new results after settings change; omitted here).

.. code-block:: python

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
                        )
                    .match(settings)
                )


