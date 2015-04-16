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

#ifndef NG_RANGEINPUTFILTER_H
#define NG_RANGEINPUTFILTER_H

#include <unity/shell/scopes/RangeInputFilterInterface.h>
#include <unity/shell/scopes/FiltersInterface.h>
#include "filters.h"
#include <unity/scopes/RangeInputFilter.h>
#include <QScopedPointer>

namespace scopes_ng
{

class Q_DECL_EXPORT RangeInputFilter : public unity::shell::scopes::RangeInputFilterInterface, public FilterUpdateInterface
{
    Q_OBJECT

public:
    RangeInputFilter(unity::scopes::experimental::RangeInputFilter::SCPtr const& filter, unity::scopes::FilterState::SPtr const& filterState, unity::shell::scopes::FiltersInterface *parent = nullptr);
    QString filterId() const override;
    unity::shell::scopes::FiltersInterface::FilterType filterType() const override;
    QVariant startValue() const override;
    QVariant endValue() const override;
    void setStartValue(QVariant const& value) override;
    void setEndValue(QVariant const& value) override;
    void update(unity::scopes::FilterBase::SCPtr const& filter, unity::scopes::FilterState::SPtr const& filterState) override;

Q_SIGNALS:
    void filterStateChanged();

private:
    QString m_id;
    // TODO labels
    QString m_unitLabel;
    QVariant m_start;
    QVariant m_end;
    unity::scopes::FilterState::SPtr m_filterState;
    unity::scopes::experimental::RangeInputFilter::SCPtr m_filter;

    static bool compare(QVariant const& v1, QVariant const& v2);
};

} // namespace scopes_ng

#endif
