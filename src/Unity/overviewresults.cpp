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

// self
#include "overviewresults.h"

// local
#include "utils.h"

namespace scopes_ng {

using namespace unity;

OverviewResultsModel::OverviewResultsModel(QObject* parent)
 : unity::shell::scopes::ResultsModelInterface(parent)
{
}

void OverviewResultsModel::setResults(const QList<unity::scopes::ScopeMetadata::SPtr>& results)
{
    beginResetModel();
    m_results = results;
    endResetModel();
}

QString OverviewResultsModel::categoryId() const
{
    return QString();
}

void OverviewResultsModel::setCategoryId(const QString& id)
{
    Q_UNUSED(id);
}

int OverviewResultsModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return m_results.count();
}

int OverviewResultsModel::count() const
{
    return m_results.count();
}

int OverviewResultsModel::scopeIndex(const QString& scopeId) const
{
    std::string id(scopeId.toStdString());

    for (int i = 0; i < m_results.size(); i++) {
        if (m_results.at(i)->scope_id() == id) return i;
    }

    return -1;
}

QHash<int, QByteArray> OverviewResultsModel::roleNames() const
{
    QHash<int, QByteArray> roles(unity::shell::scopes::ResultsModelInterface::roleNames());
    roles.insert(ExtraRoles::RoleScopeId, "scopeId");

    return roles;
}

QVariant
OverviewResultsModel::data(const QModelIndex& index, int role) const
{
    scopes::ScopeMetadata* result = m_results.at(index.row()).get();

    switch (role) {
        case RoleUri: {
            scopes::CannedQuery q(result->scope_id());
            return QString::fromStdString(q.to_uri());
        }
        case RoleCategoryId:
            return QVariant();
        case RoleDndUri:
            return data(index, RoleUri);
        case RoleResult:
            return QVariant();
        case RoleTitle:
            return QString::fromStdString(result->display_name());
        case RoleArt: {
            std::string art;
            try {
                art = result->art();
            } catch (...) {
                try {
                    art = result->icon();
                } catch (...) {
                    // no icon, oh well
                }
            }
            return QString::fromStdString(art);
        }
        case RoleSubtitle:
            return QVariant();
        case RoleMascot:
            return QVariant();
        case RoleEmblem:
            return QVariant();
        case RoleAttributes:
            return QVariant();
        case RoleSummary:
            return QVariant();
        case RoleBackground:
            return QVariant();
        case RoleScopeId:
            return QString::fromStdString(result->scope_id());
        default:
            return QVariant();
    }
}

} // namespace scopes_ng
