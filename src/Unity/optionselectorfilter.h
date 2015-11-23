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

#ifndef NG_OPTIONSELECTORFILTER_H
#define NG_OPTIONSELECTORFILTER_H

#include <unity/shell/scopes/OptionSelectorFilterInterface.h>
#include <unity/shell/scopes/FiltersInterface.h>
#include "filters.h"
#include <unity/scopes/OptionSelectorFilter.h>
#include "optionselectoroptions.h"
#include <QScopedPointer>

namespace scopes_ng
{

class Q_DECL_EXPORT OptionSelectorFilter : public unity::shell::scopes::OptionSelectorFilterInterface, public FilterUpdateInterface
{
    Q_OBJECT

public:
    OptionSelectorFilter(unity::scopes::OptionSelectorFilter::SCPtr const& filter, unity::scopes::FilterState::SPtr const& filterState, unity::shell::scopes::FiltersInterface *parent = nullptr);
    QString filterId() const override;
    QString title() const override;
    unity::shell::scopes::FiltersInterface::FilterType filterType() const override;
    QString label() const override;
    bool multiSelect() const override;
    unity::shell::scopes::OptionSelectorOptionsInterface* options() const override;
    void update(unity::scopes::FilterBase::SCPtr const& filter, unity::scopes::FilterState::SPtr const& filterState) override;
    bool isActive() const override;
    QString filterTag() const override;
    void reset() override;

Q_SIGNALS:
    void filterStateChanged();

protected Q_SLOTS:
    void onOptionChecked(const QString& id, bool checked);

private:
    QString m_id;
    QString m_title;
    bool m_multiSelect;
    QString m_label;
    QScopedPointer<OptionSelectorOptions> m_options;
    std::weak_ptr<unity::scopes::FilterState> m_filterState;
    unity::scopes::OptionSelectorFilter::SCPtr m_filter;
};

} // namespace scopes_ng

#endif
