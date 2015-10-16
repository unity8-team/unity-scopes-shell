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
#include <QQmlEngine>
#include <QDebug>

#include <unity/scopes/OptionSelectorFilter.h>

namespace scopes_ng
{

Filters::Filters(unity::shell::scopes::ScopeInterface *parent)
    : ModelUpdate(parent)
{
}

int Filters::rowCount(const QModelIndex&) const
{
    return m_filters.count();
}

QVariant Filters::data(const QModelIndex& index, int role) const
{
    if (index.row() >= m_filters.count())
    {
        return QVariant();
    }

    switch (role)
    {
        case unity::shell::scopes::FiltersInterface::Roles::RoleFilterId:
            return m_filters.at(index.row())->filterId();
        case Qt::DisplayRole:
        case unity::shell::scopes::FiltersInterface::Roles::RoleFilterType:
            return m_filters.at(index.row())->filterType();
        case unity::shell::scopes::FiltersInterface::Roles::RoleFilter:
            return QVariant::fromValue(m_filters.at(index.row()).data());
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

void Filters::update(QList<unity::scopes::FilterBase::SCPtr> const& filters, unity::scopes::FilterState const& filterState, bool containsDepartments)
{
    m_filterState.reset(new unity::scopes::FilterState(filterState));

    // Primary filter needs to be handled separately and not inserted into the main filters model,
    bool hasPrimaryFilter = containsDepartments;
    QList<unity::scopes::FilterBase::SCPtr> inFilters;
    for (auto f: filters) {
        // we can only have one primary filter
        if (f->display_hints() && unity::scopes::FilterBase::DisplayHints::Primary && !hasPrimaryFilter) {
            hasPrimaryFilter = true;
            //
            const bool hadSamePrimaryFilterBefore = m_primaryFilter && m_primaryFilter->filterId() == QString::fromStdString(f->id()) && m_primaryFilter->filterType() == getFilterType(f);
            if (hadSamePrimaryFilterBefore) {
                auto shellFilter = dynamic_cast<FilterUpdateInterface*>(m_primaryFilter.data());
                if (shellFilter) {
                    shellFilter->update(f, m_filterState);
                } else {
                    // this should never happen
                    qCritical() << "Failed to cast filter" << m_primaryFilter->filterId() << "to FilterUpdateInterface";
                }
            } else {
                // we didn't have primary filter before, or it changed substantially, so recreate it
                m_primaryFilter = createFilterObject(f);
                Q_EMIT primaryFilterChanged();
            }
        } else {
            inFilters.append(f);
        }
    }

    // Did we have primary filter before but not now?
    if (!hasPrimaryFilter && m_primaryFilter != nullptr) {
        m_primaryFilter.reset();
        Q_EMIT primaryFilterChanged();
    }

    syncModel(inFilters, m_filters,
            // key function for scopes api filter
            [](const unity::scopes::FilterBase::SCPtr& f) -> QString { return QString::fromStdString(f->id()); },
            // key function for shell api filter
            [](const QSharedPointer<unity::shell::scopes::FilterBaseInterface>& f) -> QString { return f->filterId(); },
            // factory function
            [this](const unity::scopes::FilterBase::SCPtr& f) -> QSharedPointer<unity::shell::scopes::FilterBaseInterface> {
                return createFilterObject(f);
                },
            // filter update function
            [this](int, const unity::scopes::FilterBase::SCPtr &f1, const QSharedPointer<unity::shell::scopes::FilterBaseInterface>& f2) -> bool {
                if (f2->filterId() != QString::fromStdString(f1->id()) || f2->filterType() != getFilterType(f1))
                {
                    return false;
                }
                auto shellFilter = dynamic_cast<FilterUpdateInterface*>(f2.data());
                if (shellFilter) {
                    shellFilter->update(f1, m_filterState);
                }
                return true;
            });
}

QSharedPointer<unity::shell::scopes::FilterBaseInterface> Filters::createFilterObject(unity::scopes::FilterBase::SCPtr const& filter)
{
    QSharedPointer<unity::shell::scopes::FilterBaseInterface> filterObj;
    if (filter->filter_type() == "option_selector")
    {
        unity::scopes::OptionSelectorFilter::SCPtr optfilter = std::dynamic_pointer_cast<unity::scopes::OptionSelectorFilter const>(filter);
        filterObj = QSharedPointer<unity::shell::scopes::FilterBaseInterface>(new scopes_ng::OptionSelectorFilter(optfilter, m_filterState, this));
    }

    if (filterObj)
    {
        QQmlEngine::setObjectOwnership(filterObj.data(), QQmlEngine::CppOwnership);
        connect(filterObj.data(), SIGNAL(filterStateChanged()), this, SIGNAL(filterStateChanged()));
    }
    else
    {
        qWarning() << "Unsupported filter type:" << QString::fromStdString(filter->filter_type());
    }

    return filterObj;
}

unity::shell::scopes::FiltersInterface::FilterType Filters::getFilterType(unity::scopes::FilterBase::SCPtr const& filter)
{
    if (typeid(*filter) == typeid(unity::scopes::OptionSelectorFilter))
    {
        return unity::shell::scopes::FiltersInterface::FilterType::OptionSelectorFilter;
    }
    return unity::shell::scopes::FiltersInterface::FilterType::Invalid;
}

unity::scopes::FilterState Filters::filterState() const
{
    if (m_filterState) {
        return *m_filterState;
    }
    return unity::scopes::FilterState();
}

QSharedPointer<unity::shell::scopes::FilterBaseInterface> Filters::primaryFilter() const
{
    return m_primaryFilter;
}

}
