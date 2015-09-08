/*
 * Copyright (C) 2014 Canonical, Ltd.
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


#ifndef NG_OVERVIEW_CATEGORIES_H
#define NG_OVERVIEW_CATEGORIES_H

#include <unity/scopes/ScopeMetadata.h>

#include "categories.h"

namespace scopes_ng
{

struct ScopesCategoryData;
class OverviewResultsModel;

class Q_DECL_EXPORT OverviewCategories : public scopes_ng::Categories
{
    Q_OBJECT

public:
    explicit OverviewCategories(QObject* parent = 0);
    virtual ~OverviewCategories();

    void setSurfacingMode(bool isSurfacing);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    void setOtherScopes(const QList<unity::scopes::ScopeMetadata::SPtr>& scopes, const QMap<QString, QString>& scopeIdToName);
    void setFavoriteScopes(const QList<unity::scopes::ScopeMetadata::SPtr>& scopes, const QMap<QString, QString>& scopeIdToName);
    void updateOtherScopes(const QList<unity::scopes::ScopeMetadata::SPtr>& scopes, const QMap<QString, QString>& scopeIdToName);
    void updateFavoriteScopes(const QList<unity::scopes::ScopeMetadata::SPtr>& scopes, const QMap<QString, QString>& scopeIdToName);

private:
    bool m_isSurfacing;

    QList<QSharedPointer<ScopesCategoryData>> m_surfaceCategories;
    QScopedPointer<OverviewResultsModel> m_otherScopes;
    QScopedPointer<OverviewResultsModel> m_favoriteScopes;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::OverviewCategories*)

#endif // NG_OVERVIEW_CATEGORIES_H
