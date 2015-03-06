/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * Authors:
 *  Pawel Stolowski <pawel.stolowski@canonical.com>
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
 */

// Self
#include "filters.h"
#include "optionselectorfilter.h"
#include <QSet>
#include <QMap>
#include <QDebug>

#include <unity/scopes/OptionSelectorFilter.h>

namespace scopes_ng
{

Filters::Filters(unity::shell::scopes::ScopeInterface *parent)
    : unity::shell::scopes::FiltersInterface(parent)
{
}

int Filters::count() const
{
    return m_filters.count();
}

QVariant Filters::data(const QModelIndex& index, int role) const
{
    if (index.row() > m_filters.count())
    {
        return QVariant();
    }

    switch (role)
    {
        case unity::shell::scopes::FiltersInterface::Roles::RoleFilterId:
            return m_filters.at(index.row())->id();
        case Qt::DisplayRole:
        case unity::shell::scopes::FiltersInterface::Roles::RoleFilterType:
            return m_filters.at(index.row())->filterType();
        case unity::shell::scopes::FiltersInterface::Roles::RoleFilter:
            return QVariant::fromValue(m_filters.at(index.row()));
        default:
            break;
    };

    return QVariant();
}

void Filters::clear()
{
    if (m_filters.size() > 0)
    {
        beginResetModel();
        m_filters.clear();
        endResetModel();
    }
}

void Filters::update(QList<unity::scopes::FilterBase::SCPtr> const& filters, unity::scopes::FilterState const& filterState)
{
    int pos = 0;
    QMap<QString, int> newFilters;
    for (auto filter: filters)
    {
        newFilters[QString::fromStdString(filter->id())] = pos++;
    }

    int row = 0;
    QSet<QString> oldFilters; // lookup for filters that were already displayed
    // iterate over old filters, remove filters that are not present anymore
    for (auto it = m_filters.begin(); it != m_filters.end();)
    {
        const QString id = (*it)->property("id").toString();
        if (!newFilters.contains(id))
        {
            beginRemoveRows(QModelIndex(), row, row);
            it = m_filters.erase(it);
            qDebug() << "Removing filter with id" << id << "at row" << row;
            endRemoveRows();
        }
        else
        {
            oldFilters.insert(id);
            ++it;
            ++row;
        }
    }

    // iterate over new filters, insert new filters
    row = 0;
    for (auto const& filter: filters)
    {
        if (!oldFilters.contains(QString::fromStdString(filter->id())))
        {
            auto filterObj = createFilterObject(filter);
            if (filterObj)
            {
                beginInsertRows(QModelIndex(), row, row);
                qDebug() << "Inserting new filter with id" << filterObj->id() << "in row" << row;
                m_filters.insert(row, filterObj);
                endInsertRows();
            }
            else
            {
                qWarning() << "Could not create filter object for filter" << QString::fromStdString(filter->id());
            }
        }
        row++;
    }

    // move filters if position changed
    for (int i = 0; i<m_filters.size(); )
    {
        auto const id = m_filters[i]->id();
        int pos = newFilters.value(id, -1);
        if (pos >= 0 && pos != i) {
            beginMoveRows(QModelIndex(), i, i, QModelIndex(), pos + (pos > i ? 1 : 0));
            qDebug() << "Moving filter" << id << "from row" << i << "to" << pos;
            m_filters.move(i, pos);
            endMoveRows();
            continue;
        }
        i++;
    }

    // at this point m_filters has filters with same ids as received filters
    // and in same order, but filter types or content may have changed
    for (int i = 0; i<filters.size(); ++i)
    {
        if (oldFilters.contains(QString::fromStdString(filters[i]->id())))
        {
            auto existingFilter = m_filters.value(i);
            auto receivedFilter = filters.at(i);
            if (existingFilter->filterType() == QString::fromStdString(receivedFilter->filter_type()))
            {
                qDebug() << "Updating filter" << existingFilter->id() << "of type" << existingFilter->filterType();
                existingFilter->update(receivedFilter, filterState);
            }
            else
            {
                // id same but type changed
                qDebug() << "Recreating filter" << existingFilter->id() << "of type" << existingFilter->filterType() << ", new type" <<
                    QString::fromStdString(receivedFilter->filter_type()) << "at row" << i;
                m_filters[i] = createFilterObject(receivedFilter);
                Q_EMIT dataChanged(index(i, 0), index(i, 0)); // or beginRemoveRows & beginInsertRows ?
            }
        }
    }
}

unity::shell::scopes::FilterBaseInterface* Filters::createFilterObject(unity::scopes::FilterBase::SCPtr const& filter)
{
    if (filter->filter_type() == "option_selector")
    {
        unity::scopes::OptionSelectorFilter::SCPtr optfilter = std::dynamic_pointer_cast<unity::scopes::OptionSelectorFilter const>(filter);
        return new OptionSelectorFilter(optfilter);
    }
    return nullptr;
}

}
