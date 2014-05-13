/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *  Michał Sawicz <michal.sawicz@canonical.com>
 *  Michal Hruby <michal.hruby@canonical.com>
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

// self
#include "previewwidgetmodel.h"

// local
#include "utils.h"

// Qt
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace scopes_ng
{

using namespace unity;

PreviewWidgetModel::PreviewWidgetModel(QObject* parent)
{
    setParent(parent);
}

void PreviewWidgetModel::insertWidget(QSharedPointer<PreviewWidgetData> const& widget, int position)
{
    int insertPos = position >= 0 && position <= m_previewWidgets.count() ? position : m_previewWidgets.count();
    beginInsertRows(QModelIndex(), insertPos, insertPos);

    m_previewWidgets.insert(insertPos, widget);

    endInsertRows();
}

void PreviewWidgetModel::addWidgets(QList<QSharedPointer<PreviewWidgetData>> const& widgetList)
{
    if (widgetList.size() == 0) return;

    beginInsertRows(QModelIndex(), m_previewWidgets.count(), m_previewWidgets.size() + widgetList.size() - 1);

    m_previewWidgets.append(widgetList);

    endInsertRows();
}

void PreviewWidgetModel::adoptWidgets(QList<QSharedPointer<PreviewWidgetData>> const& widgetList)
{
    beginResetModel();

    m_previewWidgets.clear();
    m_previewWidgets.append(widgetList);

    endResetModel();
}

void PreviewWidgetModel::clearWidgets()
{
    beginRemoveRows(QModelIndex(), 0, m_previewWidgets.count() - 1);
    m_previewWidgets.clear();
    endRemoveRows();
}

bool PreviewWidgetModel::widgetChanged(PreviewWidgetData* widget)
{
    for (int i = 0; i < m_previewWidgets.size(); i++) {
        if (m_previewWidgets[i].data() == widget) {
            QModelIndex changedIndex(index(i));
            QVector<int> changedRoles;
            changedRoles.append(PreviewWidgetModel::RoleProperties);
            dataChanged(changedIndex, changedIndex, changedRoles);

            return true;
        }
    }

    return false;
}

int PreviewWidgetModel::rowCount(const QModelIndex&) const
{
    return m_previewWidgets.size();
}

QVariant PreviewWidgetModel::data(const QModelIndex& index, int role) const
{
    auto widget_data = m_previewWidgets.at(index.row());
    switch (role) {
        case RoleWidgetId:
            return widget_data->id;
        case RoleType:
            return widget_data->type;
        case RoleProperties:
            return widget_data->data;
        default:
            return QVariant();
    }
}

} // namespace scopes_ng
