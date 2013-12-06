/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
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


#ifndef NG_CATEGORY_RESULTS_H
#define NG_CATEGORY_RESULTS_H

#include <QAbstractListModel>

#include <scopes/CategorisedResult.h>

namespace scopes_ng {

class Q_DECL_EXPORT ResultsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_ENUMS(Roles)

    Q_PROPERTY(QString categoryId READ categoryId WRITE setCategoryId NOTIFY categoryIdChanged)
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    explicit ResultsModel(QObject* parent = 0);

    enum Roles {
        RoleUri,
        RoleIconHint,
        RoleCategory,
        RoleTitle,
        RoleComment,
        RoleDndUri,
        RoleMetadata,
        RoleRendererHints
    };

    int rowCount(const QModelIndex& parent) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void addResults(QList<std::shared_ptr<unity::api::scopes::CategorisedResult>> const&);
    void clearResults();

    /* getters */
    QString categoryId() const;
    int count() const;

    /* setters */
    void setCategoryId(QString const& id);

Q_SIGNALS:
    void categoryIdChanged();
    void countChanged();

private:
    QHash<int, QByteArray> m_roles;
    QList<std::shared_ptr<unity::api::scopes::CategorisedResult>> m_results;
    QString m_categoryId;
};

} // namespace scopes_ng

#endif // NG_CATEGORY_RESULTS_H
