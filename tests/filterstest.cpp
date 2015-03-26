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
namespace uss = unity::shell::scopes;

class FiltersTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:

    void init()
    {
        filtersModel.reset(new Filters());

        f1 = unity::scopes::OptionSelectorFilter::create("f1", "Filter1", false);
        f1o1 = f1->add_option("o1", "Option1");
        f1o2 = f1->add_option("o2", "Option2");

        f2 = unity::scopes::OptionSelectorFilter::create("f2", "Filter2", false);
        f2o1 = f2->add_option("o1", "Option1");
        f2o2 = f2->add_option("o2", "Option2");
    }

    void testFiltersModelInit()
    {
        unity::scopes::FilterState filterState;

        QSignalSpy filtersSpy(filtersModel.data(), SIGNAL(rowsInserted(const QModelIndex&, int, int)));
        {
            QList<unity::scopes::FilterBase::SCPtr> backendFilters;
            backendFilters.append(f1);

            f1->update_state(filterState, f1o1, true);

            filtersModel->update(backendFilters, filterState);
        }

        QCOMPARE(filtersSpy.count(), 1); // 1 filter object added

        // check filters model data
        auto idx = filtersModel->index(0, 0);
        QCOMPARE(filtersModel->rowCount(), 1);
        QCOMPARE(filtersModel->data(idx, unity::shell::scopes::FiltersInterface::Roles::RoleFilterId).toString(), QString("f1"));
        QCOMPARE(filtersModel->data(idx, unity::shell::scopes::FiltersInterface::Roles::RoleFilterType).toInt(),
                static_cast<int>(unity::shell::scopes::FiltersInterface::FilterType::OptionSelectorFilter));

        // get filter object from the model
        auto opf = filtersModel->data(idx, uss::FiltersInterface::Roles::RoleFilter).value<OptionSelectorFilter*>();
        QVERIFY(opf != nullptr);
        QCOMPARE(opf->filterId(), QString("f1"));
        QCOMPARE(opf->label(), QString("Filter1"));
        QVERIFY(!opf->multiSelect());
    }

    void testFiltersModelInsert()
    {
        unity::scopes::FilterState filterState;

        QSignalSpy rowsInsertedSignal(filtersModel.data(), SIGNAL(rowsInserted(const QModelIndex&, int, int)));
        QSignalSpy rowsRemovedSignal(filtersModel.data(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)));
        QSignalSpy rowsMovedSignal(filtersModel.data(), SIGNAL(rowsMoved(const QModelIndex&, int, int, const QModelIndex&, int)));

        {
            QList<unity::scopes::FilterBase::SCPtr> backendFilters;
            backendFilters.append(f1);

            filtersModel->update(backendFilters, filterState);
        }

        QCOMPARE(rowsMovedSignal.count(), 0);
        QCOMPARE(rowsRemovedSignal.count(), 0);
        QCOMPARE(rowsInsertedSignal.count(), 1);
        // verify arguments of rowsInsertedSignal
        {
            auto args = rowsInsertedSignal.takeFirst();
            auto row = args.at(1).toInt();
            QCOMPARE(row, 0);
        }

        rowsInsertedSignal.clear();

        // insert new filter
        {
            QList<unity::scopes::FilterBase::SCPtr> backendFilters;
            backendFilters.append(f1);
            backendFilters.append(f2);

            filtersModel->update(backendFilters, filterState);
        }

        QCOMPARE(rowsMovedSignal.count(), 0);
        QCOMPARE(rowsRemovedSignal.count(), 0);
        QCOMPARE(rowsInsertedSignal.count(), 1);
        // verify arguments of rowsInsertedSignal
        {
            auto args = rowsInsertedSignal.takeFirst();
            auto row = args.at(1).toInt();
            QCOMPARE(row, 1);
        }

        auto idx1 = filtersModel->index(0, 0);
        auto idx2 = filtersModel->index(1, 0);
        QCOMPARE(filtersModel->rowCount(), 2);
        QCOMPARE(filtersModel->data(idx1, unity::shell::scopes::FiltersInterface::Roles::RoleFilterId).toString(), QString("f1"));
        QCOMPARE(filtersModel->data(idx2, unity::shell::scopes::FiltersInterface::Roles::RoleFilterId).toString(), QString("f2"));
        QCOMPARE(filtersModel->data(idx2, unity::shell::scopes::FiltersInterface::Roles::RoleFilterType).toInt(),
                static_cast<int>(unity::shell::scopes::FiltersInterface::FilterType::OptionSelectorFilter));

    }

    void testFiltersModelMove()
    {
        unity::scopes::FilterState filterState;
        {
            QList<unity::scopes::FilterBase::SCPtr> backendFilters;
            backendFilters.append(f1);
            backendFilters.append(f2);

            filtersModel->update(backendFilters, filterState);
        }

        QSignalSpy rowsInsertedSignal(filtersModel.data(), SIGNAL(rowsInserted(const QModelIndex&, int, int)));
        QSignalSpy rowsRemovedSignal(filtersModel.data(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)));
        QSignalSpy rowsMovedSignal(filtersModel.data(), SIGNAL(rowsMoved(const QModelIndex&, int, int, const QModelIndex&, int)));

        // change filters positions
        {
            QList<unity::scopes::FilterBase::SCPtr> backendFilters;

            backendFilters.append(f2);
            backendFilters.append(f1);

            filtersModel->update(backendFilters, filterState);
        }

        QCOMPARE(rowsInsertedSignal.count(), 0); // no change
        QCOMPARE(rowsRemovedSignal.count(), 0);
        QCOMPARE(rowsMovedSignal.count(), 1);

        QCOMPARE(filtersModel->rowCount(), 2);

        auto idx1 = filtersModel->index(0, 0);
        auto idx2 = filtersModel->index(1, 0);
        QCOMPARE(filtersModel->data(idx1, unity::shell::scopes::FiltersInterface::Roles::RoleFilterId).toString(), QString("f2"));
        QCOMPARE(filtersModel->data(idx2, unity::shell::scopes::FiltersInterface::Roles::RoleFilterId).toString(), QString("f1"));

        // verify arguments of rowsMovedSignal
        {
            auto args = rowsMovedSignal.takeFirst();
            auto srcRow = args.at(1).toInt();
            auto dstRow = args.at(4).toInt();
            QCOMPARE(srcRow, 0);
            QCOMPARE(dstRow, 2);
        }
    }

    void testFiltersModelRemove()
    {
        unity::scopes::FilterState filterState;
        {
            QList<unity::scopes::FilterBase::SCPtr> backendFilters;
            backendFilters.append(f2);
            backendFilters.append(f1);

            filtersModel->update(backendFilters, filterState);
        }

        QSignalSpy rowsInsertedSignal(filtersModel.data(), SIGNAL(rowsInserted(const QModelIndex&, int, int)));
        QSignalSpy rowsRemovedSignal(filtersModel.data(), SIGNAL(rowsRemoved(const QModelIndex&, int, int)));
        QSignalSpy rowsMovedSignal(filtersModel.data(), SIGNAL(rowsMoved(const QModelIndex&, int, int, const QModelIndex&, int)));

        // remove a filter
        {
            QList<unity::scopes::FilterBase::SCPtr> backendFilters;
            backendFilters.append(f2); // filter1 not present (removed)

            filtersModel->update(backendFilters, filterState);
        }

        QCOMPARE(rowsInsertedSignal.count(), 0);
        QCOMPARE(rowsMovedSignal.count(), 0);
        QCOMPARE(rowsRemovedSignal.count(), 1);
        // verify arguments of rowsRemovedSignal
        {
            auto args = rowsRemovedSignal.takeFirst();
            auto first = args.at(1).toInt();
            auto last = args.at(2).toInt();
            QCOMPARE(first, 1);
            QCOMPARE(last, 1);
        }

        QCOMPARE(filtersModel->rowCount(), 1);

        auto idx1 = filtersModel->index(0, 0);
        QCOMPARE(filtersModel->data(idx1, unity::shell::scopes::FiltersInterface::Roles::RoleFilterId).toString(), QString("f2"));
    }

private:
    QScopedPointer<Filters> filtersModel;
    unity::scopes::OptionSelectorFilter::SPtr f1, f2;
    unity::scopes::FilterOption::SCPtr f1o1, f1o2, f2o1, f2o2;
};

QTEST_GUILESS_MAIN(FiltersTest)
#include <filterstest.moc>
