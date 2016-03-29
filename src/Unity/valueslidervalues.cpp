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
#include "valuesliderfilter.h"
#include <utility>

namespace scopes_ng
{

ValueSliderValues::ValueSliderValues(ValueSliderFilter *parent)
    : ModelUpdate(parent)
{
}

void ValueSliderValues::update(const unity::scopes::ValueSliderLabels& values, int min, int max)
{
    unity::scopes::ValueLabelPairList labels;
    labels.push_back(std::make_pair(min, values.min_label()));
    for (auto const v: values.extra_labels())
    {
        labels.push_back(v);
    }
    labels.push_back(std::make_pair(max, values.max_label()));
    syncModel(labels, m_values,
            [](const unity::scopes::ValueLabelPair& p) -> int { return p.first; },
            [](const QSharedPointer<QPair<int, QString>>& p) -> int { return p->first; },
            [](const unity::scopes::ValueLabelPair& p) -> QSharedPointer<QPair<int, QString>> {
                return QSharedPointer<QPair<int, QString>>(new QPair<int, QString>(p.first, QString::fromStdString(p.second)));
                },
            [this](int row, const unity::scopes::ValueLabelPair& v1, const QSharedPointer<QPair<int, QString>>& v2) -> bool {
                if (v1.first != v2->first) {
                    return false;
                }

                if (v1.second != v2->second.toStdString()) {
                    Q_EMIT dataChanged(index(row, 0), index(row, 0), { unity::shell::scopes::ValueSliderValuesInterface::Roles::RoleLabel });
                }
                return true;
            });
}

int ValueSliderValues::rowCount(const QModelIndex&) const
{
    return m_values.count();
}

QVariant ValueSliderValues::data(const QModelIndex& index, int role) const
{
    if (index.row() >= m_values.count())
    {
        return QVariant();
    }
    switch (role)
    {
        case Qt::DisplayRole:
        case RoleValue:
            return QVariant(m_values.at(index.row())->first);
        case RoleLabel:
            return QVariant(m_values.at(index.row())->second);
        default:
            return QVariant();
    }
}

}
