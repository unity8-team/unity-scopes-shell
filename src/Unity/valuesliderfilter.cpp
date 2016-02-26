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

#include "valuesliderfilter.h"
#include "utils.h"
#include <cmath>
#include <functional>
#include <QQmlEngine>
#include <QDebug>

using namespace unity::scopes;

namespace scopes_ng
{

ValueSliderFilter::ValueSliderFilter(unity::scopes::ValueSliderFilter::SCPtr const& filter, unity::scopes::FilterState::SPtr const& filterState, unity::shell::scopes::FiltersInterface *parent)
    : unity::shell::scopes::ValueSliderFilterInterface(parent),
    m_id(QString::fromStdString(filter->id())),
    m_title(QString::fromStdString(filter->title())),
    m_min(filter->min()),
    m_max(filter->max()),
    m_values(new ValueSliderValues(this)),
    m_filterState(filterState),
    m_filter(filter)
{
    QQmlEngine::setObjectOwnership(m_values.data(), QQmlEngine::CppOwnership);
    m_value = (filter->has_value(*filterState) ? filter->value(*filterState) : filter->default_value());
}

QString ValueSliderFilter::filterId() const
{
    return m_id;
}

QString ValueSliderFilter::title() const
{
    return m_title;
}

unity::shell::scopes::FiltersInterface::FilterType ValueSliderFilter::filterType() const
{
    return unity::shell::scopes::FiltersInterface::FilterType::ValueSliderFilter;
}

double ValueSliderFilter::value() const
{
    return m_value;
}

void ValueSliderFilter::setValue(double value)
{
    if (auto state = m_filterState.lock()) {
        if (value != m_value) {
            m_filter->update_state(*state, m_value = value);

            Q_EMIT valueChanged();
            Q_EMIT filterStateChanged();
        }
    }
}

double ValueSliderFilter::minValue() const
{
    return m_min;
}

double ValueSliderFilter::maxValue() const
{
    return m_max;
}

unity::shell::scopes::ValueSliderValuesInterface* ValueSliderFilter::values() const
{
    return m_values.data();
}

void ValueSliderFilter::update(unity::scopes::FilterBase::SCPtr const& filter, unity::scopes::FilterState::SPtr const& filterState)
{
    m_filterState = filterState;

    unity::scopes::ValueSliderFilter::SCPtr valueslider = std::dynamic_pointer_cast<unity::scopes::ValueSliderFilter const>(filter);
    if (!valueslider) {
        qWarning() << "ValueSliderFilter::update(): Unexpected filter" << QString::fromStdString(filter->id()) << "of type" << QString::fromStdString(filter->filter_type());
        return;
    }

    m_filter = valueslider;

    if (valueslider->title() != m_title.toStdString())
    {
        m_title = QString::fromStdString(valueslider->title());
        Q_EMIT titleChanged();
    }

    const double value = (valueslider->has_value(*filterState) ? valueslider->value(*filterState) : valueslider->default_value());
    if (value != m_value) {
        m_value = value;
        Q_EMIT valueChanged();
    }

    if (std::abs(valueslider->min() - m_min) < 0.0000001f) {
        m_min = valueslider->min();
        Q_EMIT minValueChanged();
    }

    if (std::abs(valueslider->max() - m_max) < 0.0000001f) {
        m_max = valueslider->max();
        Q_EMIT maxValueChanged();
    }

    m_values->update(valueslider->labels(), valueslider->min(), valueslider->max());
}

bool ValueSliderFilter::isActive() const
{
    if (auto state = m_filterState.lock()) {
        if (m_filter->has_value(*state) && m_filter->value(*state) != m_filter->default_value()) {
            return true;
        }
    }
    return false;
}

QString ValueSliderFilter::filterTag() const
{
    return ""; // slider filter can't be a primary navigation filter
}

void ValueSliderFilter::reset()
{
    setValue(m_filter->default_value());
}

}
