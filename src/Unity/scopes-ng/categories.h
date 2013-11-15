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


#ifndef NG_CATEGORIES_H
#define NG_CATEGORIES_H

#include <QAbstractListModel>
#include <QSet>
#include <QTimer>

#include <scopes/Category.h>

namespace scopes_ng
{

class Categories : public QAbstractListModel
{
    Q_OBJECT

    Q_ENUMS(Roles)

public:
    explicit Categories(QObject* parent = 0);

    enum Roles {
        RoleCategoryId,
        RoleName,
        RoleIcon,
        RoleRenderer,
        RoleContentType,
        RoleRendererHint,
        RoleProgressSource,
        RoleHints,
        RoleResults,
        RoleCount
    };

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent) const override;

    void registerCategory(unity::api::scopes::Category::SCPtr category);

private Q_SLOTS:

private:
    QHash<int, QByteArray> m_roles;
    QList<unity::api::scopes::Category::SCPtr> m_categories;
};

} // namespace scopes_ng

#endif // NG_CATEGORIES_H
