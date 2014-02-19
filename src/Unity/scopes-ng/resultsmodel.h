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
#include <QHash>

#include <unity/scopes/CategorisedResult.h>
#include <unordered_map>

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
        RoleCategoryId,
        RoleDndUri,
        RoleResult,
        // card components
        RoleTitle,
        RoleArt,
        RoleSubtitle,
        RoleMascot,
        RoleEmblem,
        RoleOldPrice,
        RolePrice,
        RoleAltPrice,
        RoleRating,
        RoleAltRating,
        RoleSummary,
        RoleBackground
    };

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE QVariant get(int row) const;

    void addResults(QList<std::shared_ptr<unity::scopes::CategorisedResult>> const&);
    void clearResults();

    /* getters */
    QString categoryId() const;
    int count() const;

    /* setters */
    void setCategoryId(QString const& id);
    void setComponentsMapping(QHash<QString, QString> const& mapping);

Q_SIGNALS:
    void categoryIdChanged();
    void countChanged();

private:
    QVariant componentValue(unity::scopes::CategorisedResult const* result, std::string const& fieldName) const;

    QHash<int, QByteArray> m_roles;
    std::unordered_map<std::string, std::string> m_componentMapping;
    QList<std::shared_ptr<unity::scopes::CategorisedResult>> m_results;
    QString m_categoryId;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(std::shared_ptr<unity::scopes::Result>)

#endif // NG_CATEGORY_RESULTS_H
