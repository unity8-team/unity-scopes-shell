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

FilterGroupWidget::FilterGroupWidget(unity::scopes::FilterState::SPtr const& filterState, unity::shell::scopes::FiltersInterface *parent)
{
}

QString FilterGroupWidget::filterId() const
{
}

QString FilterGroupWidget::title() const
{
}

unity::shell::scopes::FiltersInterface::FilterType FilterGroupWidget::filterType() const
{
}

unity::shell::scopes::FiltersInterface* FilterGroupWidget::filters() const
{
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
