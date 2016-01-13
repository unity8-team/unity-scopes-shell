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

#include <QSignalSpy>
#include <QScopedPointer>
#include <QTest>
#include <scopes.h>
#include <scope.h>
#include "filters.h"
#include "categories.h"
#include "optionselectorfilter.h"
#include "rangeinputfilter.h"
#include "valuesliderfilter.h"
#include <scope-harness/scope-harness.h>
#include <scope-harness/registry/pre-existing-registry.h>
#include <scope-harness/test-utils.h>
#include <unity/shell/scopes/CategoriesInterface.h>

#include <unity/scopes/OptionSelectorFilter.h>
#include <unity/scopes/RangeInputFilter.h>
#include <unity/scopes/ValueSliderFilter.h>

using namespace unity::scopeharness;
using namespace unity::scopeharness::registry;
using namespace scopes_ng;

namespace uss = unity::shell::scopes;
namespace sh = unity::scopeharness;
namespace shr = unity::scopeharness::registry;
namespace shv = unity::scopeharness::view;
namespace sc = unity::scopes;

// FIXME: use scope harness wrapper once it exposes filters
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

    void cleanupTestCase()
    {
    }

    void init()
    {
    }

    void cleanup()
    {
    }

    void testBasic()
    {
        auto resultsView = m_harness->resultsView();
        resultsView->setActiveScope("mock-scope-filters");
        resultsView->setQuery("");
        //TODO
    }
};

QTEST_GUILESS_MAIN(FiltersScopeHarnessTest)
#include <filtersharnesstest.moc>
