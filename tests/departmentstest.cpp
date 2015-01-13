/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authors:
 *  Michal Hruby <michal.hruby@canonical.com>
 */

#include <QObject>
#include <QTest>

#include <scope-harness/matcher/category-matcher.h>
#include <scope-harness/matcher/category-list-matcher.h>
#include <scope-harness/matcher/department-matcher.h>
#include <scope-harness/matcher/result-matcher.h>
#include <scope-harness/scope-harness.h>
#include <scope-harness/test-utils.h>

using namespace std;

namespace sc = unity::scopes;
namespace sh = unity::scopeharness;
namespace shm = unity::scopeharness::matcher;
namespace shr = unity::scopeharness::registry;
namespace shv = unity::scopeharness::view;
namespace ss = unity::shell::scopes;
namespace ng = scopes_ng;

class DepartmentsTest : public QObject
{
    Q_OBJECT
private:
    sh::ScopeHarness::UPtr m_harness;

    shv::ResultsView::SPtr m_resultsView;

private Q_SLOTS:
    void initTestCase()
    {
        qputenv("UNITY_SCOPES_NO_WAIT_LOCATION", "1");
        m_harness = sh::ScopeHarness::newFromScopeList(
            shr::CustomRegistry::Parameters({
                TEST_DATA_DIR "mock-scope-departments/mock-scope-departments.ini",
                TEST_DATA_DIR "mock-scope-double-nav/mock-scope-double-nav.ini",
                TEST_DATA_DIR "mock-scope-departments-flipflop/mock-scope-departments-flipflop.ini"
            })
        );
        m_resultsView = m_harness->resultsView();
    }

    void cleanupTestCase()
    {
        m_resultsView.reset();
        m_harness.reset();
    }

    void testNoDepartments()
    {
        m_resultsView->setActiveScope("mock-scope-departments");
        m_resultsView->setQuery("foo");

        QVERIFY(!m_resultsView->hasNavigation());
        QVERIFY(!m_resultsView->hasAltNavigation());
    }

    void testRootDepartment()
    {
        m_resultsView->setActiveScope("mock-scope-departments");
        m_resultsView->setQuery("");

        QVERIFY(m_resultsView->hasNavigation());
        QVERIFY(!m_resultsView->hasAltNavigation());
        QVERIFY(m_resultsView->departmentId().empty());

        auto departments = m_resultsView->browseDepartment();
        QCOMPARE(m_resultsView->departmentId(), string());

        QVERIFY_MATCHRESULT(
            shm::DepartmentMatcher()
                .hasExactly(5)
                .label("All departments")
                .allLabel(string())
                .parentId(string())
                .parentLabel(string())
                .isRoot(true)
                .isHidden(false)
                .child(shm::ChildDepartmentMatcher("books")
                    .label("Books")
                    .hasChildren(true)
                    .isActive(false)
                )
                .child(shm::ChildDepartmentMatcher("movies"))
                .child(shm::ChildDepartmentMatcher("electronics"))
                .child(shm::ChildDepartmentMatcher("home"))
                .child(shm::ChildDepartmentMatcher("toys")
                    .label("Toys, Children & Baby")
                    .hasChildren(true)
                    .isActive(false)
                )
                .match(departments)
        );
    }

    void testChildDepartmentModel()
    {
        m_resultsView->setActiveScope("mock-scope-departments");
        m_resultsView->setQuery("");

        auto departments = m_resultsView->browseDepartment("toys");
        QCOMPARE(m_resultsView->departmentId(), string("toys"));

        QVERIFY_MATCHRESULT(
            shm::DepartmentMatcher()
                .id("toys")
                .label("Toys, Children & Baby")
                .allLabel(string())
                .parentId(string())
                .parentLabel(string("All departments"))
                .isRoot(false)
                .hasExactly(2)
                .match(departments)
        );
    }

    void testLeafActivationUpdatesModel()
    {
        m_resultsView->setActiveScope("mock-scope-departments");
        m_resultsView->setQuery("");

        auto books = m_resultsView->browseDepartment("books");
        QCOMPARE(m_resultsView->departmentId(), string("books"));
        QVERIFY(!books.isRoot());

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .hasAtLeast(1)
                .mode(shm::CategoryListMatcher::Mode::starts_with)
                .category(shm::CategoryMatcher("cat1")
                    .hasAtLeast(1)
                    .mode(shm::CategoryMatcher::Mode::starts_with)
                    .result(shm::ResultMatcher("test:uri")
                        .title("result for: \"\", department \"books\"")
                    )
                )
                .match(m_resultsView->categories())
        );

        auto booksAudio = m_resultsView->browseDepartment("books-audio");
        QVERIFY(!booksAudio.isRoot());

        QVERIFY_MATCHRESULT(
            shm::CategoryListMatcher()
                .hasAtLeast(1)
                .mode(shm::CategoryListMatcher::Mode::starts_with)
                .category(shm::CategoryMatcher("cat1")
                    .hasAtLeast(1)
                    .mode(shm::CategoryMatcher::Mode::starts_with)
                    .result(shm::ResultMatcher("test:uri")
                        .title("result for: \"\", department \"books-audio\"")
                    )
                )
                .match(m_resultsView->categories())
        );

        QVERIFY_MATCHRESULT(
            shm::DepartmentMatcher()
                .mode(shm::DepartmentMatcher::Mode::by_id)
                .child(shm::ChildDepartmentMatcher("books-audio"))
                .match(books)
        );
    }

    // This test has always been broken
    void testGoingBack()
    {
//        sh::performSearch(m_scope, QString("x"));
//
//        QCOMPARE(m_scope->currentNavigationId(), QString(""));
//        QSignalSpy spy(m_scope.data(), SIGNAL(searchInProgressChanged()));
//        QScopedPointer<ss::NavigationInterface> navModel(m_scope->getNavigation(QString("books")));
//        m_scope->setNavigationState(navModel->navigationId(), false);
//        QVERIFY(spy.wait());
//        QCOMPARE(m_scope->searchInProgress(), false);
//        QScopedPointer<ss::NavigationInterface> departmentModel(m_scope->getNavigation(QString("books")));
//        QCOMPARE(departmentModel->isRoot(), false);
//
//        // get the root again without actually loading the department
//        departmentModel.reset(m_scope->getNavigation(departmentModel->parentNavigationId()));
//        QCOMPARE(departmentModel->isRoot(), true);
//        QEXPECT_FAIL("", "We have the department in cache, to it kind of is loaded", Continue);
//        QCOMPARE(departmentModel->loaded(), false);
    }

    void testIncompleteTreeOnLeaf()
    {
        m_resultsView->setActiveScope("mock-scope-departments");
        m_resultsView->setQuery("");

        auto toys = m_resultsView->browseDepartment("toys");
        QCOMPARE(m_resultsView->departmentId(), string("toys"));
        QCOMPARE(toys.size(), 2ul);

        auto toysGames = m_resultsView->browseDepartment("toys-games");
        QCOMPARE(m_resultsView->departmentId(), string("toys-games"));
        QCOMPARE(toysGames.size(), 0ul);

        // after getting the parent department model, it should still have
        // all the leaves, even though the leaf served just itself
        auto toys2 = m_resultsView->browseDepartment("toys");
        QCOMPARE(m_resultsView->departmentId(), string("toys"));
        QCOMPARE(toys2.size(), 2ul);
    }

    void testDoubleNavigation()
    {
        m_resultsView->setActiveScope("mock-scope-double-nav");
        m_resultsView->setQuery("");
        auto root = m_resultsView->browseDepartment();

        QVERIFY(m_resultsView->hasNavigation());
        QVERIFY(m_resultsView->hasAltNavigation());
        QVERIFY(m_resultsView->departmentId().empty());
        QCOMPARE(m_resultsView->altDepartmentId(), string("featured"));

        auto sortOrder = m_resultsView->browseAltDepartment();

        QVERIFY_MATCHRESULT(
            shm::DepartmentMatcher()
                .id(string())
                .label("Sort Order")
                .allLabel(string())
                .parentId(string())
                .parentLabel(string())
                .isRoot(true)
                .isHidden(true)
                .hasExactly(3)
                .child(shm::ChildDepartmentMatcher("featured")
                    .label("Featured")
                    .hasChildren(false)
                    .isActive(true)
                )
                .child(shm::ChildDepartmentMatcher("top"))
                .child(shm::ChildDepartmentMatcher("best")
                    .label("Best sellers")
                    .hasChildren(false)
                    .isActive(false)
                )
                .match(sortOrder)
        );
    }

    void testDoubleNavChangeActive()
    {
        m_resultsView->setActiveScope("mock-scope-double-nav");
        m_resultsView->setQuery("");
        auto root = m_resultsView->browseDepartment();

        QCOMPARE(m_resultsView->altDepartmentId(), string("featured"));

        auto sortOrder = m_resultsView->browseAltDepartment();

        QVERIFY_MATCHRESULT(
            shm::DepartmentMatcher()
                .id(string())
                .label("Sort Order")
                .hasExactly(3)
                .child(shm::ChildDepartmentMatcher("featured"))
                .child(shm::ChildDepartmentMatcher("top")
                    .isActive(false)
                )
                .child(shm::ChildDepartmentMatcher("best"))
                .match(sortOrder)
        );

        QVERIFY_MATCHRESULT(
            shm::DepartmentMatcher()
                .id(string("top"))
                .hasExactly(0)
                .match(m_resultsView->browseAltDepartment("top"))
        );
    }

    void testDepartmentDissapear()
    {
        m_resultsView->setActiveScope("mock-scope-departments-flipflop");
        m_resultsView->setQuery("");
        auto root = m_resultsView->browseDepartment();

        QVERIFY(m_resultsView->hasNavigation());
        QVERIFY(!m_resultsView->hasAltNavigation());
        QVERIFY(m_resultsView->departmentId().empty());


        QCOMPARE(root.id(), string());
        QCOMPARE(root.label(), string("All departments"));
        QCOMPARE(root.allLabel(), string());
        QCOMPARE(root.parentId(), string());
        QCOMPARE(root.parentLabel(), string());
        QVERIFY(root.isRoot());
        QVERIFY(!root.isHidden());

        QCOMPARE(root.size(), 5ul);

        m_resultsView->forceRefresh();

        root = m_resultsView->browseDepartment();

        // one department removed
        QCOMPARE(root.size(), 4ul);

        {
            auto department = root.child(0);
            QCOMPARE(department.id(), string("books"));
            QCOMPARE(department.label(), string("Books"));
            QCOMPARE(department.hasChildren(), true);
            QCOMPARE(department.isActive(), false);
        }

        {
            auto department = root.child(3);
            QCOMPARE(department.id(), string("toys"));
            QCOMPARE(department.label(), string("Toys, Children & Baby"));
            QCOMPARE(department.hasChildren(), true);
            QCOMPARE(department.isActive(), false);
        }
    }

};

QTEST_GUILESS_MAIN(DepartmentsTest)
#include <departmentstest.moc>
