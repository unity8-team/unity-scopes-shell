/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 *  Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <QTest>
#include <scope-harness/scope-harness.h>
#include <scope-harness/test-utils.h>
#include <scope-harness/matcher/filter-list-matcher.h>
#include <scope-harness/matcher/option-selector-filter-matcher.h>

using namespace unity::scopeharness::registry;

namespace sh = unity::scopeharness;
namespace shr = unity::scopeharness::registry;
namespace shm = unity::scopeharness::matcher;
namespace shv = unity::scopeharness::view;

class FiltersScopeHarnessTest : public QObject
{
    Q_OBJECT
private:
    sh::ScopeHarness::UPtr m_harness;

private Q_SLOTS:

    void initTestCase()
    {
        qputenv("UNITY_SCOPES_NO_WAIT_LOCATION", "1");
        m_harness = sh::ScopeHarness::newFromScopeList(
            shr::CustomRegistry::Parameters({
                TEST_DATA_DIR "mock-scope-filters/mock-scope-filters.ini",
            })
        );
    }

    void testBasic()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope-filters");
        resultsView->setQuery("");

        // ensure we have filters
        QVERIFY_MATCHRESULT(
            shm::FilterListMatcher()
                .hasExactly(3)
                .match(resultsView->filters())
        );
    }

    void testOptionSelector()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope-filters");
        resultsView->setQuery("");

        // ensure we have filters
        QVERIFY_MATCHRESULT(
            shm::FilterListMatcher()
                .filter(
                    shm::OptionSelectorFilterMatcher("filter1")
                        .title("")
                        .hasExactly(2)
                )
                .match(resultsView->filters())
        );
    }
};

QTEST_GUILESS_MAIN(FiltersScopeHarnessTest)
#include <filtersharnesstest.moc>
