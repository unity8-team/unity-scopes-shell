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
#include <QList>
#include "filters.h"
#include "optionselectorfilter.h"

#include <unity/scopes/OptionSelectorFilter.h>

using namespace scopes_ng;

class FiltersTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void init()
    {
    }

    void initTestCase()
    {
    }

    void testFiltersInit()
    {
        namespace uss = unity::shell::scopes;

        QScopedPointer<Filters> filtersModel(new Filters());
        unity::scopes::FilterState filterState;

        QList<unity::scopes::FilterBase::SCPtr> backendFilters;
        {
            unity::scopes::OptionSelectorFilter::SPtr f1 = unity::scopes::OptionSelectorFilter::create("f1", "Filter1", false);
            auto opt1 = f1->add_option("o1", "Option1");
            auto opt2 = f1->add_option("o2", "Option2");
            backendFilters.append(f1);

            f1->update_state(filterState, opt1, true);
        }

        QSignalSpy filtersSpy(filtersModel.data(), SIGNAL(rowsInserted(const QModelIndex&, int, int)));
        filtersModel->update(backendFilters, filterState);

        QCOMPARE(filtersSpy.count(), 1); // 1 filter object added

        // check filters model data
        auto idx = filtersModel->index(0, 0);
        QCOMPARE(filtersModel->rowCount(), 1);
        QCOMPARE(filtersModel->data(idx, unity::shell::scopes::FiltersInterface::Roles::RoleFilterId).toString(), QString("f1"));
        QCOMPARE(filtersModel->data(idx, unity::shell::scopes::FiltersInterface::Roles::RoleFilterType).toInt(),
                static_cast<int>(unity::shell::scopes::FiltersInterface::FilterType::OptionSelectorFilter));

        QVERIFY(filtersModel->data(idx, uss::FiltersInterface::Roles::RoleFilter).value<OptionSelectorFilter*>() != nullptr);
    }
};

QTEST_GUILESS_MAIN(FiltersTest)
#include <filterstest.moc>
