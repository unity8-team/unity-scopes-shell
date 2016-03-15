/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#include "filtergroupwidget.h"
#include <QQmlEngine>
#include <QDebug>

namespace scopes_ng
{

FilterGroupWidget::FilterGroupWidget(QList<unity::scopes::FilterBase::SCPtr> const& filters, unity::scopes::FilterState::SPtr const& filterState, unity::shell::scopes::FiltersInterface *parent)
    : m_filters(new Filters(filterState, this))
{
    if (filters.size() > 0) {
        auto group = filters.front()->filter_group();
        Q_ASSERT(group != nullptr);
        m_id = QString::fromStdString(group->id());
        m_label = QString::fromStdString(group->label());
    }
    m_filters->update(filters, false, false);
}

QString FilterGroupWidget::filterId() const
{
    return m_id;
}

QString FilterGroupWidget::title() const
{
    return m_label;
}

unity::shell::scopes::FiltersInterface::FilterType FilterGroupWidget::filterType() const
{
    return unity::shell::scopes::FiltersInterface::FilterType::ExpandableFilterWidget;
}

unity::shell::scopes::FiltersInterface* FilterGroupWidget::filters() const
{
    return m_filters;
}

void FilterGroupWidget::update(FilterWrapper::SCPtr const& filterWrapper)
{
    m_filters->update(filterWrapper->filters, false, false);
}

void FilterGroupWidget::update(unity::scopes::FilterBase::SCPtr const& filter)
{
    // This should never happen, if it does, it's a bug
    qWarning() << "FilterGroupWidget::update(unity::scopes::FilterBase::SCPtr const&) is not supported for FilterGroupWidget";
}

void FilterGroupWidget::update(unity::scopes::FilterState::SPtr const& filterState)
{
    m_filters->update(filterState);
}

int FilterGroupWidget::activeFiltersCount() const
{
    return m_filters->activeFiltersCount();
}

QString FilterGroupWidget::filterTag() const
{
    // FilterGroup cannot have primary filters, so no tag
    return "";
}

void FilterGroupWidget::reset()
{
    m_filters->reset();
}

}
