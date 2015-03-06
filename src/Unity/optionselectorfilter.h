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

#include "OptionSelectorFilterInterface.h"
#include "FiltersInterface.h"
#include <unity/scopes/OptionSelectorFilter.h>
#include "optionselectoroptions.h"
#include <QScopedPointer>

namespace scopes_ng
{

class Q_DECL_EXPORT OptionSelectorFilter : public unity::shell::scopes::OptionSelectorFilterInterface
{
    Q_OBJECT

public:
    explicit OptionSelectorFilter(unity::scopes::OptionSelectorFilter::SCPtr const& filter, unity::shell::scopes::FiltersInterface *parent = nullptr);
    QString id() const override;
    QString filterType() const override;
    QString label() const override;
    bool multiSelect() const override;
    int count() const override;
    unity::shell::scopes::OptionSelectorOptionsInterface* options() const override;
    void update(unity::scopes::FilterBase::SCPtr const& filter, unity::scopes::FilterState const& filterState) override;

private:
    QScopedPointer<OptionSelectorOptions> m_options;
    QString m_id;
    bool m_multiSelect;
    QString m_label;
};

} // namespace scopes_ng

#endif
