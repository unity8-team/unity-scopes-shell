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
#include <functional>
#include <QDebug>

using namespace unity::scopes;

namespace scopes_ng
{

RangeInputFilter::RangeInputFilter(unity::scopes::RangeInputFilter::SCPtr const& filter, unity::scopes::FilterState::SPtr const& filterState, unity::shell::scopes::FiltersInterface *parent)
    : unity::shell::scopes::RangeInputFilterInterface(parent),
    m_id(QString::fromStdString(filter->id())),
    m_title(QString::fromStdString(filter->title())),
    m_startPrefixLabel(QString::fromStdString(filter->start_prefix_label())),
    m_startPostfixLabel(QString::fromStdString(filter->start_postfix_label())),
    m_centralLabel(QString::fromStdString(filter->central_label())),
    m_endPrefixLabel(QString::fromStdString(filter->end_prefix_label())),
    m_endPostfixLabel(QString::fromStdString(filter->end_postfix_label())),
    m_defaultStart(filter->default_start_value()),
    m_defaultEnd(filter->default_end_value()),
    m_filterState(filterState),
    m_filter(filter)
{
}

QString RangeInputFilter::filterId() const
{
    return m_id;
}

QString RangeInputFilter::title() const
{
    return m_title;
}

unity::shell::scopes::FiltersInterface::FilterType RangeInputFilter::filterType() const
{
    return unity::shell::scopes::FiltersInterface::FilterType::RangeInputFilter;
}

double RangeInputFilter::startValue() const
{
    if (m_start.which() == Variant::Double) {
        return m_start.get_double();
    }
    qWarning() << "Requested startValue for filter" << m_id << ", but value is not set";
    return 0.0f;
}

double RangeInputFilter::endValue() const
{
    if (m_end.which() == Variant::Double) {
        return m_end.get_double();
    }
    qWarning() << "Requested endValue for filter" << m_id << ", but value is not set";
    return 0.0f;
}

void RangeInputFilter::setStartValue(double value)
{
    const unity::scopes::Variant newValue(value);
    setStartValue(newValue);
}

void RangeInputFilter::setEndValue(double value)
{
    const unity::scopes::Variant newValue(value);
    setEndValue(newValue);
}

void RangeInputFilter::labelChange(std::string const& srcLabel, QString& destLabel, std::function<void()> const& emitLabelChangeSignal)
{
    auto const srclbl = QString::fromStdString(srcLabel);
    if (srclbl != destLabel) {
        destLabel = srclbl;
        emitLabelChangeSignal();
    }
}

void RangeInputFilter::update(unity::scopes::FilterBase::SCPtr const& filter, unity::scopes::FilterState::SPtr const& filterState)
{
    m_filterState = filterState;

    unity::scopes::RangeInputFilter::SCPtr rangefilter = std::dynamic_pointer_cast<unity::scopes::RangeInputFilter const>(filter);
    if (!rangefilter) {
        qWarning() << "RangeInputFilter::update(): Unexpected filter" << QString::fromStdString(filter->id()) << "of type" << QString::fromStdString(filter->filter_type());
        return;
    }

    m_filter = rangefilter;

    if (rangefilter->title() != m_title.toStdString())
    {
        m_title = QString::fromStdString(rangefilter->title());
        Q_EMIT titleChanged();
    }

    labelChange(m_filter->start_prefix_label(), m_startPrefixLabel, [this]() { Q_EMIT startPrefixLabelChanged(); });
    labelChange(m_filter->start_postfix_label(), m_startPostfixLabel, [this]() { Q_EMIT startPostfixLabelChanged(); });
    labelChange(m_filter->central_label(), m_centralLabel, [this]() { Q_EMIT centralLabelChanged(); });
    labelChange(m_filter->end_prefix_label(), m_endPrefixLabel, [this]() { Q_EMIT endPrefixLabelChanged(); });
    labelChange(m_filter->end_postfix_label(), m_endPostfixLabel, [this]() { Q_EMIT endPostfixLabelChanged(); });

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
        // check if current value from filter state is equal to default value
        if (m_filter->has_start_value(*state) && !compare(m_filter->start_value(*state), m_filter->default_start_value())) {
            return true;
        }
        if (m_filter->has_end_value(*state) && !compare(m_filter->end_value(*state), m_filter->default_end_value())) {
            return true;
        }
    }
    return false;
}

QString RangeInputFilter::filterTag() const
{
    return ""; // range input filter can't be a primary navigation filter
}

QString RangeInputFilter::startPrefixLabel() const
{
    return m_startPrefixLabel;
}

QString RangeInputFilter::startPostfixLabel() const
{
    return m_startPostfixLabel;
}

QString RangeInputFilter::centralLabel() const
{
    return m_centralLabel;
}

QString RangeInputFilter::endPrefixLabel() const
{
    return m_endPrefixLabel;
}

QString RangeInputFilter::endPostfixLabel() const
{
    return m_endPostfixLabel;
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
    setStartValue(Variant::null());
}

void RangeInputFilter::eraseEndValue()
{
    setEndValue(Variant::null());
}

void RangeInputFilter::setStartValue(Variant const& value)
{
    if (auto state = m_filterState.lock()) {
        try {
            if (!compare(value, m_start)) {
                m_start = value;

                m_filter->update_state(*state, m_start, m_end);

                if (value.is_null()) {
                    Q_EMIT hasStartValueChanged();
                }
                Q_EMIT startValueChanged();
                Q_EMIT filterStateChanged();
            }
        }
        catch (std::exception const& err)
        {
            // this is ok, it's user input and we may get partial input
            qWarning() << "Could not set start value of filter" << m_id << ":" << QString::fromStdString(err.what());
        }
    }
}

void RangeInputFilter::setEndValue(Variant const& value)
{
    if (auto state = m_filterState.lock()) {
        try {
            if (!compare(value, m_end)) {
                m_end = value;

                m_filter->update_state(*state, m_start, m_end);

                if (value.is_null()) {
                    Q_EMIT hasEndValueChanged();
                }
                Q_EMIT endValueChanged();
                Q_EMIT filterStateChanged();
            }
        }
        catch (std::exception const& err)
        {
            // this is ok, it's user input and we may get partial input
            qWarning() << "Could not set start value of filter" << m_id << ":" << QString::fromStdString(err.what());
        }
    }
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

bool RangeInputFilter::compare(double v1, Variant const& v2)
{
    if (v2.which() == Variant::Double) {
        return std::abs(v1 - v2.get_double()) < 0.0000001f;
    }
    return false;
}

void RangeInputFilter::reset()
{
    // Filters::onFilterStateChanged will delay actual refresh,
    // so it's ok to make two independent updates and emit two signals
    setStartValue(m_filter->default_start_value());
    setEndValue(m_filter->default_end_value());
}

}
