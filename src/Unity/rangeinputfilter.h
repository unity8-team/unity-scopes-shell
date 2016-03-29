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
    RangeInputFilter(unity::scopes::RangeInputFilter::SCPtr const& filter, unity::scopes::FilterState::SPtr const& filterState, unity::shell::scopes::FiltersInterface *parent = nullptr);
    QString filterId() const override;
    QString title() const override;
    unity::shell::scopes::FiltersInterface::FilterType filterType() const override;
    double startValue() const override;
    double endValue() const override;

    QString startPrefixLabel() const override;
    QString startPostfixLabel() const override;
    QString centralLabel() const override;
    QString endPrefixLabel() const override;
    QString endPostfixLabel() const override;

    void setStartValue(double value) override;
    void setEndValue(double value) override;

    bool hasStartValue() const override;
    bool hasEndValue() const override;

    Q_INVOKABLE void eraseStartValue() override;
    Q_INVOKABLE void eraseEndValue() override;

    void update(unity::scopes::FilterBase::SCPtr const& filter) override;
    void update(unity::scopes::FilterState::SPtr const& filterState) override;
    bool isActive() const override;
    QString filterTag() const override;
    void reset() override;

Q_SIGNALS:
    void filterStateChanged();

private:
    void setStartValue(unity::scopes::Variant const& value);
    void setEndValue(unity::scopes::Variant const& value);
    static void labelChange(std::string const& srcLabel, QString& destLabel, std::function<void()> const& emitLabelChangeSignal);

    QString m_id;
    QString m_title;
    QString m_startPrefixLabel;
    QString m_startPostfixLabel;
    QString m_centralLabel;
    QString m_endPrefixLabel;
    QString m_endPostfixLabel;
    unity::scopes::Variant m_defaultStart;
    unity::scopes::Variant m_defaultEnd;
    unity::scopes::Variant m_start;
    unity::scopes::Variant m_end;
    std::weak_ptr<unity::scopes::FilterState> m_filterState;
    unity::scopes::RangeInputFilter::SCPtr m_filter;

    static bool compare(double v1, unity::scopes::Variant const& v2);
    static bool compare(unity::scopes::Variant const& v1, unity::scopes::Variant const& v2);
};

} // namespace scopes_ng

#endif
