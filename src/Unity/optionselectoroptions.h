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

#ifndef NG_OPTIONSELECTOROPTIONS_H
#define NG_OPTIONSELECTOROPTIONS_H

#include <unity/shell/scopes/OptionSelectorOptionsInterface.h>
#include <unity/scopes/FilterOption.h>
#include "modelupdate.h"
#include <QSharedPointer>
#include <QList>
#include <list>
#include <set>
#include <functional>

namespace scopes_ng
{

class OptionSelectorFilter;
struct OptionSelectorOption;

class Q_DECL_EXPORT OptionSelectorOptions :
    public ModelUpdate<unity::shell::scopes::OptionSelectorOptionsInterface,
                       std::list<unity::scopes::FilterOption::SCPtr>,
                       QList<QSharedPointer<OptionSelectorOption>>>
{
    Q_OBJECT

public:
    OptionSelectorOptions(OptionSelectorFilter *parent,
            std::list<unity::scopes::FilterOption::SCPtr> const& options,
            std::set<unity::scopes::FilterOption::SCPtr> const& activeOptions);
    void update(const std::list<unity::scopes::FilterOption::SCPtr>& options);
    void update(const std::set<unity::scopes::FilterOption::SCPtr>& activeOptions, bool allow_defaults);
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE void setChecked(int row, bool checked) override;

Q_SIGNALS:
    void optionChecked(const QString&, bool);

private:
    QList<QSharedPointer<OptionSelectorOption>> m_options;
};

} // namespace scopes_ng

#endif
