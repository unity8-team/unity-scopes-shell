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

#ifndef NG_VALUESLIDERVALUES_H
#define NG_VALUESLIDERVALUES_H

#include <unity/shell/scopes/ValueSliderValuesInterface.h>
#include "modelupdate.h"
#include <QPair>
#include <QSharedPointer>
#include <QString>
#include <unity/scopes/ValueSliderLabels.h>

namespace scopes_ng
{

class ValueSliderFilter;

class Q_DECL_EXPORT ValueSliderValues :
    public ModelUpdate<unity::shell::scopes::ValueSliderValuesInterface,
        unity::scopes::experimental::ValueLabelPairList,
        QList<QSharedPointer<QPair<int, QString>>>,
        int
    >
{
    Q_OBJECT

public:
    explicit ValueSliderValues(ValueSliderFilter *parent = nullptr);
    void update(const unity::scopes::experimental::ValueSliderLabels& values, int min, int max);
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

private:
    QList<QSharedPointer<QPair<int, QString>>> m_values;
};

}

#endif
