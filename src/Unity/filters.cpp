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
    //: unity::shell::scopes::FiltersInterface(parent)
{
}

int Filters::rowCount(const QModelIndex&) const
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
    m_filterState.reset(new unity::scopes::FilterState(filterState));

    syncModel(filters, m_filters,
            [](const unity::scopes::FilterBase::SCPtr& f) -> QString { return QString::fromStdString(f->id()); },
            [](const QSharedPointer<unity::shell::scopes::FilterBaseInterface>& f) -> QString { return f->id(); },
            [this](const unity::scopes::FilterBase::SCPtr& f) -> QSharedPointer<unity::shell::scopes::FilterBaseInterface> {
                return createFilterObject(f);
                },
            [this](const unity::scopes::FilterBase::SCPtr &f1, const QSharedPointer<unity::shell::scopes::FilterBaseInterface>& f2) -> bool {
                if (f2->id() != QString::fromStdString(f1->id()) || f2->filterType() != QString::fromStdString(f1->filter_type()))
                {
                    return false;
                }
                f2->update(f1, m_filterState);
                return true;
            });
}

QSharedPointer<unity::shell::scopes::FilterBaseInterface> Filters::createFilterObject(unity::scopes::FilterBase::SCPtr const& filter)
{
    if (filter->filter_type() == "option_selector")
    {
        unity::scopes::OptionSelectorFilter::SCPtr optfilter = std::dynamic_pointer_cast<unity::scopes::OptionSelectorFilter const>(filter);
        auto filterObj = QSharedPointer<unity::shell::scopes::FilterBaseInterface>(new OptionSelectorFilter(optfilter, m_filterState, this));
        connect(filterObj.data(), SIGNAL(filterStateChanged()), this, SIGNAL(filterStateChanged()));
    }
    return QSharedPointer<unity::shell::scopes::FilterBaseInterface>();
}

unity::scopes::FilterState Filters::filterState()
{
    if (m_filterState) {
        return *m_filterState;
    }
    return unity::scopes::FilterState();
}

}
