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
#include <scope-harness/registry/pre-existing-registry.h>
#include <scope-harness/test-utils.h>
#include <unity/shell/scopes/CategoriesInterface.h>
#include <unity/shell/scopes/OptionSelectorFilterInterface.h>

#include <unity/scopes/OptionSelectorFilter.h>
#include <unity/scopes/RangeInputFilter.h>
#include <unity/scopes/ValueSliderFilter.h>

using namespace unity::scopeharness;
using namespace unity::scopeharness::registry;
using namespace scopes_ng;
namespace uss = unity::shell::scopes;

// FIXME: use scope harness wrapper once it exposes filters
class FiltersEndToEndTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void initTestCase()
    {
        m_registry.reset(new PreExistingRegistry(TEST_RUNTIME_CONFIG));
        m_registry->start();
    }

    void cleanupTestCase()
    {
        m_registry.reset();
    }

    void init()
    {
        const QStringList favs {"scope://mock-scope-filters"};
        TestUtils::setFavouriteScopes(favs);

        m_scopes.reset(new Scopes(nullptr));

        // wait till the registry spawns
        QSignalSpy spy(m_scopes.data(), SIGNAL(loadedChanged()));
        QVERIFY(spy.wait());
        QCOMPARE(m_scopes->loaded(), true);

        // get scope proxy
        m_scope = m_scopes->getScopeById("mock-scope-filters");
        QVERIFY(m_scope != nullptr);
        m_scope->setActive(true);
    }

    void cleanup()
    {
        m_scopes.reset();
        m_scope.reset();
    }

    void testBasic()
    {
        TestUtils::performSearch(m_scope, "");

        QCOMPARE(m_scope->primaryNavigationTag(), QString(""));
        QVERIFY(m_scope->primaryNavigationFilter() == nullptr);

        auto filters = m_scope->filters();
        QVERIFY(filters != nullptr);
        QCOMPARE(filters->rowCount(), 3);

        auto idx = filters->index(0, 0);
        QCOMPARE(filters->data(idx, unity::shell::scopes::FiltersInterface::Roles::RoleFilterId).toString(), QString("f1"));
        idx = filters->index(1, 0);
        QCOMPARE(filters->data(idx, unity::shell::scopes::FiltersInterface::Roles::RoleFilterId).toString(), QString("f2"));
        idx = filters->index(2, 0);
        QCOMPARE(filters->data(idx, unity::shell::scopes::FiltersInterface::Roles::RoleFilterId).toString(), QString("f3"));
    }

    void testOptionSelectorFilter()
    {
        TestUtils::performSearch(m_scope, "");

        auto categories = m_scope->categories();
        QVERIFY(categories != nullptr);
        auto results = categories->data(categories->index(0, 0),
                Categories::RoleResultsSPtr).value<QSharedPointer<unity::shell::scopes::ResultsModelInterface>>();
        QVERIFY(results != nullptr);

        auto filters = m_scope->filters();
        QVERIFY(filters != nullptr);
        QCOMPARE(filters->rowCount(), 3);

        auto idx = filters->index(0, 0);
        auto f1 = filters->data(idx, uss::FiltersInterface::Roles::RoleFilter).value<OptionSelectorFilter*>();
        QVERIFY(f1 != nullptr);

        // check options
        auto opts = f1->options();
        QVERIFY(opts != nullptr);
        QCOMPARE(opts->rowCount(), 2);

        QCOMPARE(f1->filterId(), QString("f1"));
        QCOMPARE(f1->label(), QString("Filter1"));
        QVERIFY(!f1->multiSelect());

        idx = opts->index(0, 0);
        QCOMPARE(opts->data(idx, uss::OptionSelectorOptionsInterface::Roles::RoleOptionId).toString(), QString("o1"));
        QVERIFY(!opts->data(idx, uss::OptionSelectorOptionsInterface::Roles::RoleOptionChecked).toBool());
        idx = opts->index(1, 0);
        QCOMPARE(opts->data(idx, uss::OptionSelectorOptionsInterface::Roles::RoleOptionId).toString(), QString("o2"));
        QVERIFY(!opts->data(idx, uss::OptionSelectorOptionsInterface::Roles::RoleOptionChecked).toBool());

        // select option 1
        opts->setChecked(0, true);
        TestUtils::waitForFilterStateChange(m_scope);
        TestUtils::waitForSearchFinish(m_scope);
        QCOMPARE(filters, m_scope->filters());
        QCOMPARE(results->data(results->index(0, 0), unity::shell::scopes::ResultsModelInterface::RoleTitle).toString(), QString("result for option o1"));

        // select option 2
        opts->setChecked(1, true);
        TestUtils::waitForFilterStateChange(m_scope);
        TestUtils::waitForSearchFinish(m_scope);
        QCOMPARE(filters, m_scope->filters());
        QCOMPARE(results->data(results->index(0, 0), unity::shell::scopes::ResultsModelInterface::RoleTitle).toString(), QString("result for option o2"));

        // deselect option 2
        opts->setChecked(1, false);
        TestUtils::waitForFilterStateChange(m_scope);
        TestUtils::waitForSearchFinish(m_scope);
        QCOMPARE(results->data(results->index(0, 0), unity::shell::scopes::ResultsModelInterface::RoleTitle).toString(), QString("result for: \"\""));

        QCOMPARE(filters, m_scope->filters());
    }

    void testPrimaryFilter()
    {
        TestUtils::performSearch(m_scope, "test_primary_filter");

        auto filters = m_scope->filters();
        QVERIFY(filters != nullptr);
        QCOMPARE(filters->rowCount(), 2);

        auto idx = filters->index(0, 0);
        QCOMPARE(filters->data(idx, unity::shell::scopes::FiltersInterface::Roles::RoleFilterId).toString(), QString("f2"));
        idx = filters->index(1, 0);
        QCOMPARE(filters->data(idx, unity::shell::scopes::FiltersInterface::Roles::RoleFilterId).toString(), QString("f3"));

        auto f1 = dynamic_cast<unity::shell::scopes::OptionSelectorFilterInterface *>(m_scope->primaryNavigationFilter());
        QVERIFY(f1 != nullptr);
        QCOMPARE(f1->filterId(), QString("f1"));

        auto opts = f1->options();

        // select option 1
        opts->setChecked(0, true);
        TestUtils::waitForFilterStateChange(m_scope);
        TestUtils::waitForSearchFinish(m_scope);

        QCOMPARE(m_scope->primaryNavigationTag(), QString("Option1"));
    }

    void testRangeInputFilter()
    {
        TestUtils::performSearch(m_scope, "");

        auto categories = m_scope->categories();
        QVERIFY(categories != nullptr);
        auto results = categories->data(categories->index(0, 0),
                Categories::RoleResultsSPtr).value<QSharedPointer<unity::shell::scopes::ResultsModelInterface>>();
        QVERIFY(results != nullptr);

        auto filters = m_scope->filters();
        QVERIFY(filters != nullptr);
        QCOMPARE(filters->rowCount(), 3);

        auto idx = filters->index(1, 0);
        auto f2 = filters->data(idx, uss::FiltersInterface::Roles::RoleFilter).value<RangeInputFilter*>();
        QVERIFY(f2 != nullptr);

        QCOMPARE(f2->hasStartValue(), true);
        QCOMPARE(f2->hasEndValue(), false);

        {
            f2->setStartValue(111.0f);
            TestUtils::waitForFilterStateChange(m_scope);
            TestUtils::waitForSearchFinish(m_scope);

            QCOMPARE(filters, m_scope->filters());
            QCOMPARE(f2->hasStartValue(), true);
            QCOMPARE(f2->hasEndValue(), false);

            auto resultIdx = filters->index(0, 0);
            QCOMPARE(results->data(resultIdx, unity::shell::scopes::ResultsModelInterface::RoleTitle).toString(), QString("result for range: 111.000000 - ***"));
        }

        {
            QCOMPARE(f2, filters->data(idx, uss::FiltersInterface::Roles::RoleFilter).value<RangeInputFilter*>());
            f2->setEndValue(300.5f);
            TestUtils::waitForFilterStateChange(m_scope);
            TestUtils::waitForSearchFinish(m_scope);

            QCOMPARE(f2->hasStartValue(), true);
            QCOMPARE(f2->hasEndValue(), true);

            auto resultIdx = filters->index(0, 0);
            QCOMPARE(filters, m_scope->filters());
            QCOMPARE(results->data(resultIdx, unity::shell::scopes::ResultsModelInterface::RoleTitle).toString(), QString("result for range: 111.000000 - 300.500000"));
        }

        // erase start value, end value still present
        {
            f2->eraseStartValue();
            TestUtils::waitForFilterStateChange(m_scope);
            TestUtils::waitForSearchFinish(m_scope);

            QCOMPARE(filters, m_scope->filters());
            QCOMPARE(f2->hasStartValue(), false);
            QCOMPARE(f2->hasEndValue(), true);

            auto resultIdx = filters->index(0, 0);
            QCOMPARE(results->data(resultIdx, unity::shell::scopes::ResultsModelInterface::RoleTitle).toString(), QString("result for range: *** - 300.500000"));
        }
    }

    void testValueSliderFilter()
    {
        TestUtils::performSearch(m_scope, "");

        auto categories = m_scope->categories();
        QVERIFY(categories != nullptr);
        auto results = categories->data(categories->index(0, 0),
                Categories::RoleResultsSPtr).value<QSharedPointer<unity::shell::scopes::ResultsModelInterface>>();
        QVERIFY(results != nullptr);

        auto filters = m_scope->filters();
        QVERIFY(filters != nullptr);
        QCOMPARE(filters->rowCount(), 3);

        auto idx = filters->index(2, 0);
        auto f3 = filters->data(idx, uss::FiltersInterface::Roles::RoleFilter).value<ValueSliderFilter*>();
        QVERIFY(f3 != nullptr);

        QCOMPARE(static_cast<int>(f3->minValue()), 1);
        QCOMPARE(static_cast<int>(f3->maxValue()), 99);
        QCOMPARE(static_cast<int>(f3->value()), 50);

        auto valuesModel = f3->values();
        QVERIFY(valuesModel != nullptr);

        QCOMPARE(valuesModel->rowCount(), 3);
        QCOMPARE(valuesModel->data(valuesModel->index(0, 0), uss::ValueSliderValuesInterface::Roles::RoleValue).toInt(), 1);
        QCOMPARE(valuesModel->data(valuesModel->index(0, 0), uss::ValueSliderValuesInterface::Roles::RoleLabel).toString(), QString("Min"));
        QCOMPARE(valuesModel->data(valuesModel->index(1, 0), uss::ValueSliderValuesInterface::Roles::RoleValue).toInt(), 33);
        QCOMPARE(valuesModel->data(valuesModel->index(1, 0), uss::ValueSliderValuesInterface::Roles::RoleLabel).toString(), QString("One third"));
        QCOMPARE(valuesModel->data(valuesModel->index(2, 0), uss::ValueSliderValuesInterface::Roles::RoleValue).toInt(), 99);
        QCOMPARE(valuesModel->data(valuesModel->index(2, 0), uss::ValueSliderValuesInterface::Roles::RoleLabel).toString(), QString("Max"));

        f3->setValue(75);
        TestUtils::waitForFilterStateChange(m_scope);
        TestUtils::waitForSearchFinish(m_scope);

        QCOMPARE(filters->rowCount(), 3);

        // filter object shouldn't be recreated
        QCOMPARE(filters->data(idx, uss::FiltersInterface::Roles::RoleFilter).value<ValueSliderFilter*>(), f3);

        QCOMPARE(filters, m_scope->filters());
        QCOMPARE(static_cast<int>(f3->value()), 75);
    }

    void testFilterGroup()
    {
        TestUtils::performSearch(m_scope, "test_filter_group");

        auto filters = m_scope->filters();
        QVERIFY(filters != nullptr);
        QCOMPARE(filters->rowCount(), 2);

    }

    void testResetToDefault()
    {
        TestUtils::performSearch(m_scope, "");

        auto filters = m_scope->filters();
        QVERIFY(filters != nullptr);
        QCOMPARE(filters->rowCount(), 3);

        auto idx = filters->index(1, 0);
        auto f2 = filters->data(idx, uss::FiltersInterface::Roles::RoleFilter).value<RangeInputFilter*>();
        QVERIFY(f2 != nullptr);
        QCOMPARE(f2->startValue(), 2.0f); //QCOMPARE does fuzzy comparison for floats/doubles

        idx = filters->index(2, 0);
        auto f3 = filters->data(idx, uss::FiltersInterface::Roles::RoleFilter).value<ValueSliderFilter*>();
        QVERIFY(f3 != nullptr);
        QCOMPARE(static_cast<int>(f3->value()), 50);
        f2->setStartValue(5.0f);
        f3->setValue(75);
        TestUtils::waitForFilterStateChange(m_scope);
        TestUtils::waitForSearchFinish(m_scope);
        QCOMPARE(f2->startValue(), 5.0f);
        QCOMPARE(static_cast<int>(f3->value()), 75);

        m_scope->resetFilters();
        TestUtils::waitForFilterStateChange(m_scope);
        TestUtils::waitForSearchFinish(m_scope);
        QCOMPARE(f2->startValue(), 2.0f); //QCOMPARE does fuzzy comparison for floats/doubles
        QCOMPARE(static_cast<int>(f3->value()), 50);
    }

    void testCancel()
    {
        TestUtils::performSearch(m_scope, "");

        auto filters = m_scope->filters();
        QVERIFY(filters != nullptr);
        QCOMPARE(filters->rowCount(), 3);

        auto idx = filters->index(1, 0);
        auto f2 = filters->data(idx, uss::FiltersInterface::Roles::RoleFilter).value<RangeInputFilter*>();
        QVERIFY(f2 != nullptr);

        idx = filters->index(2, 0);
        auto f3 = filters->data(idx, uss::FiltersInterface::Roles::RoleFilter).value<ValueSliderFilter*>();
        QVERIFY(f3 != nullptr);
        QCOMPARE(static_cast<int>(f3->value()), 50);
        f2->setStartValue(5.0f);
        f3->setValue(75);
        TestUtils::waitForFilterStateChange(m_scope);
        TestUtils::waitForSearchFinish(m_scope);
        QCOMPARE(f2->startValue(), 5.0f);
        QCOMPARE(static_cast<int>(f3->value()), 75);

        m_scope->resetPrimaryNavigationTag();

        TestUtils::waitForSearchFinish(m_scope);
        QCOMPARE(f2->startValue(), 2.0f); //QCOMPARE does fuzzy comparison for floats/doubles
        QCOMPARE(static_cast<int>(f3->value()), 50);
        QCOMPARE(QString(), m_scope->primaryNavigationTag());
    }

private:
    QScopedPointer<Scopes> m_scopes;
    Scope::Ptr m_scope;
    Registry::UPtr m_registry;
};

QTEST_GUILESS_MAIN(FiltersEndToEndTest)
#include <filtersendtoendtest.moc>
