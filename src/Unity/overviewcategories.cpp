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
#include "overviewcategories.h"

// local
#include "utils.h"

namespace scopes_ng
{

#define CATEGORY_JSON R"({"schema-version":1,"template": {"category-layout":"grid","card-size":"small","overlay":true}, "components": { "title":"title", "art": {"field":"art", "aspect-ratio": 0.5}}})"

struct ScopesCategoryData
{
    QString categoryId;
    QString rawTemplate;
    QVariant rendererVar;
    QVariant componentsVar;

    ScopesCategoryData(QString const& id, QString const& jsonTemplate): categoryId(id), rawTemplate(jsonTemplate)
    {
        QJsonValue rendererTemplate;
        QJsonValue components;

        Categories::parseTemplate(jsonTemplate.toStdString(), &rendererTemplate, &components);
        rendererVar = rendererTemplate.toVariant();
        componentsVar = components.toVariant();
    }
};

OverviewCategories::OverviewCategories(QObject* parent)
    : scopes_ng::Categories(parent)
    , m_isSurfacing(true)
{
    m_surfaceCategories.append(QSharedPointer<ScopesCategoryData>(new ScopesCategoryData("favorites", CATEGORY_JSON)));
    m_surfaceCategories.append(QSharedPointer<ScopesCategoryData>(new ScopesCategoryData("all", CATEGORY_JSON)));
}

void OverviewCategories::setSurfacingMode(bool surfacingMode)
{
    if (m_isSurfacing != surfacingMode) {
        beginResetModel();
        m_isSurfacing = surfacingMode;
        endResetModel();
    }
}

int OverviewCategories::rowCount(const QModelIndex& parent) const
{
    if (m_isSurfacing) {
        return m_surfaceCategories.size();
    } else {
        return Categories::rowCount(parent);
    }
}

QVariant
OverviewCategories::data(const QModelIndex& index, int role) const
{
    if (!m_isSurfacing) {
        return Categories::data(index, role);
    }

    ScopesCategoryData* catData = m_surfaceCategories.at(index.row()).data();

    switch (role) {
        case RoleCategoryId:
            return catData->categoryId;
        case RoleName:
            return QVariant();
        case RoleIcon:
            return QVariant();
        case RoleRawRendererTemplate:
            return catData->rawTemplate;
        case RoleRenderer:
            return catData->rendererVar;
        case RoleComponents:
            return catData->componentsVar;
        case RoleHeaderLink:
            return QVariant();
        case RoleResults:
            // FIXME!
            return (index.row() < Categories::rowCount()) ? Categories::data(index, role) : QVariant();
        case RoleCount:
            return (index.row() < Categories::rowCount()) ? Categories::data(index, role) : QVariant(0);
        default:
            return QVariant();
    }
}

} // namespace scopes_ng
