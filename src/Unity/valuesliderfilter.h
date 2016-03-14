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

#ifndef NG_VALUESLIDERFILTER_H
#define NG_VALUESLIDERFILTER_H

#include <unity/shell/scopes/ValueSliderFilterInterface.h>
#include <unity/shell/scopes/FiltersInterface.h>
#include "filters.h"
#include "valueslidervalues.h"
#include <unity/scopes/ValueSliderFilter.h>

namespace scopes_ng
{

class Q_DECL_EXPORT ValueSliderFilter : public unity::shell::scopes::ValueSliderFilterInterface, public FilterUpdateInterface
{
    Q_OBJECT

public:
    ValueSliderFilter(unity::scopes::ValueSliderFilter::SCPtr const& filter, unity::scopes::FilterState::SPtr const& filterState, unity::shell::scopes::FiltersInterface *parent = nullptr);
    QString filterId() const override;
    QString title() const override;
    unity::shell::scopes::FiltersInterface::FilterType filterType() const override;
    double value() const override;

    void setValue(double value) override;
    double minValue() const override;
    double maxValue() const override;
    unity::shell::scopes::ValueSliderValuesInterface* values() const override;

    void update(unity::scopes::FilterBase::SCPtr const& filter) override;
    void update(unity::scopes::FilterState::SPtr const& filterState) override;
    int activeFiltersCount() const override;
    QString filterTag() const override;
    void reset() override;

Q_SIGNALS:
    void filterStateChanged();

private:
    QString m_id;
    QString m_title;
    double m_min;
    double m_max;
    double m_value;
    QScopedPointer<ValueSliderValues> m_values;
    std::weak_ptr<unity::scopes::FilterState> m_filterState;
    unity::scopes::ValueSliderFilter::SCPtr m_filter;
};

} // namespace scopes_ng

#endif
