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

// self
#include "categories.h"
#include <QDebug>

using namespace unity::api;

namespace scopes_ng {

Categories::Categories(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roles[Categories::RoleCategoryId] = "categoryId";
    m_roles[Categories::RoleName] = "name";
    m_roles[Categories::RoleIcon] = "icon";
    m_roles[Categories::RoleRenderer] = "renderer";
    m_roles[Categories::RoleContentType] = "contentType";
    m_roles[Categories::RoleRendererHint] = "rendererHint";
    m_roles[Categories::RoleProgressSource] = "progressSource";
    m_roles[Categories::RoleHints] = "hints";
    m_roles[Categories::RoleResults] = "results";
    m_roles[Categories::RoleCount] = "count";
}

QHash<int, QByteArray>
Categories::roleNames() const
{
    return m_roles;
}

int Categories::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return m_categories.size();
}

void Categories::registerCategory(scopes::Category::SCPtr category)
{
    // do we already have a category with this id?
    int index = -1;
    for (int i = 0; i < m_categories.size(); i++) {
        if (m_categories[i]->id() == category->id()) {
            index = i;
            break;
        }
    }
    // TODO: check if any attributes of the category changed
    if (index >= 0) {
        m_categories.replace(index, category);
    } else {
        auto last_index = m_categories.size();
        beginInsertRows(QModelIndex(), last_index, last_index);
        m_categories.append(category);
        endInsertRows();
    }
}

QVariant
Categories::data(const QModelIndex& index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (index.row() >= m_categories.size()) {
        return QVariant();
    }

    scopes::Category::SCPtr cat(m_categories[index.row()]);

    switch (role) {
        case RoleCategoryId:
            return QVariant(QString::fromStdString(cat->id()));
        case RoleName:
            return QVariant(QString("unnamed"));
        case RoleIcon:
            return QVariant(QString(""));
        case RoleRenderer:
            return QVariant(QString("default"));
        case RoleContentType:
            return QVariant(QString("default"));
        case RoleRendererHint:
            return QVariant();
        case RoleProgressSource:
            return QVariant();
        case RoleHints:
            return QVariant();
        case RoleResults:
            return QVariant();
        case RoleCount:
            return QVariant(10);
        default:
            return QVariant();
    }
}

} // namespace scopes_ng
