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

#include "valueslidervalues.h"

namespace scopes_ng
{

void ValueSliderValues::update(const unity::scopes::experimental::ValueLabelPairList& values)
{
    syncModel(values, m_values,
            [](const unity::scopes::experimental::ValueLabelPair&) -> int { return 0; },
            [](const QSharedPointer<QPair<int, QString>>) -> int { return 0; },
            [](const unity::scopes::experimental::ValueLabelPair&) -> QSharedPointer<QPair<int, QString>> { return QSharedPointer<QPair<int, QString>>(new QPair<int, QString>()); },
            [](int, const unity::scopes::experimental::ValueLabelPair&, const QSharedPointer<QPair<int, QString>>) -> bool { return false; });
}

int ValueSliderValues::rowCount(const QModelIndex& parent) const
{
    return m_values.count();
}

QVariant ValueSliderValues::data(const QModelIndex& index, int role) const
{
    return QVariant();
}

}
