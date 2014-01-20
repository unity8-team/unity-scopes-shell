/*
 * Copyright (C) 2013 Canonical, Ltd.
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


#include "preview.h"

namespace scopes_ng
{

PreviewModel::PreviewModel(QObject* parent) : QAbstractListModel(parent)
{
    m_roles[Roles::RoleWidgetId] = "widgetId";
    m_roles[Roles::RoleType] = "type";
    m_roles[Roles::RoleProperties] = "properties";
}

QVariant PreviewModel::data(const QModelIndex& index, int role) const
{
    auto widget = m_previewWidgets.at(index.row());
    switch (role) {
        case RoleWidgetId:
            return QVariant();
        case RoleType:
            return QVariant();
        case RoleProperties:
            return QVariant();
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> PreviewModel::roleNames() const
{
    return m_roles;
}

int PreviewModel::rowCount(const QModelIndex&) const
{
    return m_previewWidgets.size();
}

} // namespace scopes_ng
