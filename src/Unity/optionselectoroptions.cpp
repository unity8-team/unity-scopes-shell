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

#include "optionselectoroptions.h"
#include "optionselectorfilter.h"
#include <QDebug>

namespace scopes_ng
{

OptionSelectorOption::OptionSelectorOption(const QString& id, const QString &label)
{
}

OptionSelectorOptions::OptionSelectorOptions(OptionSelectorFilter *parent)
    : unity::shell::scopes::OptionSelectorOptionsInterface(parent)
{
}

void OptionSelectorOptions::update(std::list<unity::scopes::FilterOption::SCPtr> options, unity::scopes::FilterState const& filterState)
{
    //TODO
}

int OptionSelectorOptions::count() const
{
    return m_options.count();
}

QVariant OptionSelectorOptions::data(const QModelIndex& index, int role) const
{
    if (index.row() >= m_options.count())
    {
        return QVariant();
    }
    switch (role)
    {
        case Qt::DisplayRole:
        case RoleOption:
            return QVariant::fromValue(m_options.at(index.row()).data());
        default:
            return QVariant();
    }
}

}
