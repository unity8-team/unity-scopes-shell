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

QVariant RangeInputFilter::startValue() const
{
    return m_start;
}

QVariant RangeInputFilter::endValue() const
{
    return m_end;
}

void RangeInputFilter::setStartValue(QVariant const& value)
{
    if (!compare(value, m_start)) {
        m_start = value;

        m_filter->update_state(*m_filterState, qVariantToScopeVariant(m_start), qVariantToScopeVariant(m_end));

        Q_EMIT startValueChanged(m_start);
        Q_EMIT filterStateChanged();
    }
}

void RangeInputFilter::setEndValue(QVariant const& value)
{
    if (!compare(value, m_end)) {
        m_end = value;

        m_filter->update_state(*m_filterState, qVariantToScopeVariant(m_start), qVariantToScopeVariant(m_end));

        Q_EMIT endValueChanged(m_end);
        Q_EMIT filterStateChanged();
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

    const QVariant start = rangefilter->has_start_value(*filterState) ? QVariant(rangefilter->start_value(*filterState)) : QVariant();
    if (!compare(start, m_start)) {
        m_start = start;
        Q_EMIT startValueChanged(m_start);
    }

    const QVariant end = rangefilter->has_end_value(*filterState) ? QVariant(rangefilter->end_value(*filterState)) : QVariant();
    if (!compare(end, m_end)) {
        m_end = end;
        Q_EMIT endValueChanged(m_end);
    }
}

bool RangeInputFilter::compare(QVariant const& v1, QVariant const& v2)
{
    if (v1.type() != v2.type()) {
        return false;
    }

    return (v1.type() == QVariant::Double && std::abs(v1.value<double>() - v2.value<double>()) < 0.00001f) ||
            (v1.type() == QVariant::Int && v1.value<int>() == v2.value<int>()) ||
            (v1.isNull() && v2.isNull());
}

}
