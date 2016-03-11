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

FilterGroupWidget::FilterGroupWidget(QString const& id, QList<unity::scopes::FilterBase::SCPtr> const& filters, unity::scopes::FilterState::SPtr const& filterState, unity::shell::scopes::FiltersInterface *parent)
    : m_id(id)
{
}

QString FilterGroupWidget::filterId() const
{
    return m_id;
}

QString FilterGroupWidget::title() const
{
}

unity::shell::scopes::FiltersInterface::FilterType FilterGroupWidget::filterType() const
{
    return unity::shell::scopes::FiltersInterface::FilterType::ExpandableFilterWidget;
}

unity::shell::scopes::FiltersInterface* FilterGroupWidget::filters() const
{
}

void FilterGroupWidget::update(FilterWrapper::SCPtr const& filterWrapper)
{
    for (auto const& filter: filterWrapper->filters) {
        update(filter);
    }
}

void FilterGroupWidget::update(unity::scopes::FilterBase::SCPtr const& filter)
{
}

void FilterGroupWidget::update(unity::scopes::FilterState::SPtr const& filterState)
{
}

bool FilterGroupWidget::isActive() const
{
}

QString FilterGroupWidget::filterTag() const
{
}

void FilterGroupWidget::reset()
{
}

}
