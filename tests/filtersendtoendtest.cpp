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
#include <scope-harness/registry/pre-existing-registry.h>
#include <scope-harness/test-utils.h>
#include <unity/shell/scopes/CategoriesInterface.h>

#include <unity/scopes/OptionSelectorFilter.h>

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

        auto filters = m_scope->filters();
        QVERIFY(filters != nullptr);
        QCOMPARE(filters->rowCount(), 1);

        auto idx = filters->index(0, 0);
        QCOMPARE(filters->data(idx, unity::shell::scopes::FiltersInterface::Roles::RoleFilterId).toString(), QString("f1"));
        QCOMPARE(filters->data(idx, unity::shell::scopes::FiltersInterface::Roles::RoleFilterType).toInt(),
                static_cast<int>(unity::shell::scopes::FiltersInterface::FilterType::OptionSelectorFilter));

        auto f1 = filters->data(idx, uss::FiltersInterface::Roles::RoleFilter).value<OptionSelectorFilter*>();
        QVERIFY(f1 != nullptr);
        QCOMPARE(f1->id(), QString("f1"));
        QCOMPARE(f1->label(), QString("Filter1"));
        QVERIFY(!f1->multiSelect());

        // check options
        auto opts = f1->options();
        QVERIFY(opts != nullptr);
        QCOMPARE(opts->rowCount(), 2);

        idx = opts->index(0, 0);
        QCOMPARE(opts->data(idx, uss::OptionSelectorOptionsInterface::Roles::RoleOptionId).toString(), QString("o1"));
        QVERIFY(!opts->data(idx, uss::OptionSelectorOptionsInterface::Roles::RoleOptionChecked).toBool());
        idx = opts->index(1, 0);
        QCOMPARE(opts->data(idx, uss::OptionSelectorOptionsInterface::Roles::RoleOptionId).toString(), QString("o2"));
        QVERIFY(!opts->data(idx, uss::OptionSelectorOptionsInterface::Roles::RoleOptionChecked).toBool());
    }

    void testOptionSelected()
    {
        TestUtils::performSearch(m_scope, "");

        auto categories = m_scope->categories();
        QVERIFY(categories != nullptr);
        auto results = categories->data(categories->index(0, 0),
                Categories::RoleResultsSPtr).value<QSharedPointer<unity::shell::scopes::ResultsModelInterface>>();
        QVERIFY(results != nullptr);

        auto filters = m_scope->filters();
        QVERIFY(filters != nullptr);
        QCOMPARE(filters->rowCount(), 1);

        auto idx = filters->index(0, 0);
        auto f1 = filters->data(idx, uss::FiltersInterface::Roles::RoleFilter).value<OptionSelectorFilter*>();
        QVERIFY(f1 != nullptr);

        // check options
        auto opts = f1->options();
        QVERIFY(opts != nullptr);
        QCOMPARE(opts->rowCount(), 2);

        // select option 1
        opts->setChecked(0, true);
        TestUtils::waitForSearchFinish(m_scope);
        QCOMPARE(results->data(results->index(0, 0), unity::shell::scopes::ResultsModelInterface::RoleTitle).toString(), QString("result for option o1"));

        // select option 2
        opts->setChecked(1, true);
        TestUtils::waitForSearchFinish(m_scope);
        QCOMPARE(results->data(results->index(0, 0), unity::shell::scopes::ResultsModelInterface::RoleTitle).toString(), QString("result for option o2"));

        // deselect option 2
        opts->setChecked(1, false);
        TestUtils::waitForSearchFinish(m_scope);
        QCOMPARE(results->data(results->index(0, 0), unity::shell::scopes::ResultsModelInterface::RoleTitle).toString(), QString("result for: \"\""));
    }

private:
    QScopedPointer<Scopes> m_scopes;
    Scope::Ptr m_scope;
    Registry::UPtr m_registry;
};

QTEST_GUILESS_MAIN(FiltersEndToEndTest)
#include <filtersendtoendtest.moc>
