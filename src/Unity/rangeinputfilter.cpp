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

#include "rangeinputfilter.h"
#include "utils.h"
#include <cmath>
#include <QDebug>

using namespace unity::scopes;

namespace scopes_ng
{

RangeInputFilter::RangeInputFilter(unity::scopes::experimental::RangeInputFilter::SCPtr const& filter, unity::scopes::FilterState::SPtr const& filterState, unity::shell::scopes::FiltersInterface *parent)
    : unity::shell::scopes::RangeInputFilterInterface(parent),
    m_id(QString::fromStdString(filter->id())),
    m_filterState(filterState),
    m_filter(filter)
{
}

QString RangeInputFilter::filterId() const
{
    return m_id;
}

unity::shell::scopes::FiltersInterface::FilterType RangeInputFilter::filterType() const
{
    return unity::shell::scopes::FiltersInterface::FilterType::RangeInputFilter;
}

double RangeInputFilter::startValue() const
{
    return m_start.get_double();
}

double RangeInputFilter::endValue() const
{
    return m_end.get_double();
}

void RangeInputFilter::setStartValue(double value)
{
    const unity::scopes::Variant newValue(value);

    if (auto state = m_filterState.lock()) {
        if (!compare(newValue, m_start)) {
            m_start = newValue;

            m_filter->update_state(*state, m_start, m_end);

            Q_EMIT startValueChanged();
            Q_EMIT filterStateChanged();
        }
    }
}

void RangeInputFilter::setEndValue(double value)
{
    const unity::scopes::Variant newValue(value);

    if (auto state = m_filterState.lock()) {
        if (!compare(newValue, m_end)) {
            m_end = newValue;

            m_filter->update_state(*state, m_start, m_end);

            Q_EMIT endValueChanged();
            Q_EMIT filterStateChanged();
        }
    }
}

void RangeInputFilter::update(unity::scopes::FilterBase::SCPtr const& filter, unity::scopes::FilterState::SPtr const& filterState)
{
    m_filterState = filterState;

    unity::scopes::experimental::RangeInputFilter::SCPtr rangefilter = std::dynamic_pointer_cast<unity::scopes::experimental::RangeInputFilter const>(filter);
    if (!rangefilter) {
        qWarning() << "RangeInputFilter::update(): Unexpected filter" << QString::fromStdString(filter->id()) << "of type" << QString::fromStdString(filter->filter_type());
        return;
    }

    m_filter = rangefilter;

    const unity::scopes::Variant start = rangefilter->has_start_value(*filterState) ? Variant(rangefilter->start_value(*filterState)) : unity::scopes::Variant::null();
    if (!compare(start, m_start)) {
        m_start = start;
        Q_EMIT startValueChanged();
    }

    const unity::scopes::Variant end = rangefilter->has_end_value(*filterState) ? Variant(rangefilter->end_value(*filterState)) :  unity::scopes::Variant::null();
    if (!compare(end, m_end)) {
        m_end = end;
        Q_EMIT endValueChanged();
    }
}

bool RangeInputFilter::isActive() const
{
    if (auto state = m_filterState.lock()) {
        return (m_filter->has_start_value(*state) || m_filter->has_end_value(*state));
    }
    return false;
}

QString RangeInputFilter::filterTag() const
{
    return ""; // range input filter can't be a primary navigation filter
}

QString RangeInputFilter::startPrefixLabel() const
{
    return ""; //TODO
}

QString RangeInputFilter::startPostfixLabel() const
{
    return ""; //TODO
}

QString RangeInputFilter::centralLabel() const
{
    return ""; //TODO
}

QString RangeInputFilter::endPrefixLabel() const
{
    return ""; //TODO
}

QString RangeInputFilter::endPostfixLabel() const
{
    return ""; //TODO
}

bool RangeInputFilter::hasStartValue() const
{
    return m_start.which() == Variant::Double;
}

bool RangeInputFilter::hasEndValue() const
{
    return m_end.which() == Variant::Double;
}

void RangeInputFilter::eraseStartValue()
{
    m_start = Variant::null();
}

void RangeInputFilter::eraseEndValue()
{
    m_end = Variant::null();
}

bool RangeInputFilter::compare(Variant const& v1, Variant const& v2)
{
    if (v1 == v2) {
        return true;
    }
    if (v1.which() == Variant::Double && v2.which() == Variant::Double) {
        return std::abs(v1.get_double() - v2.get_double()) < 0.0000001f;
    }
    return false;
}

}
