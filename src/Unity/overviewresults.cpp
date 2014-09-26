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
#include "resultsmodel.h"
#include "utils.h"

#include <unity/scopes/Result.h>
#include <QSet>

namespace scopes_ng {

using namespace unity;

class FakeResult: public scopes::Result
{
public:
    FakeResult(scopes::ScopeMetadata const& metadata): Result(map_for_meta(metadata)) {}

private:
    // HACK: we need to create a fake result understood by the scopes scope, so it can create a proper preview
    static scopes::VariantMap map_for_meta(scopes::ScopeMetadata const& metadata)
    {
        scopes::VariantMap map;
        scopes::VariantMap internal;
        scopes::VariantMap attrs;

        internal["origin"] = "scopes";
        map["internal"] = internal;

        attrs["title"] = metadata.display_name();
        try {
            attrs["art"] = metadata.art();
        } catch (...) {}
        try {
            attrs["icon"] = metadata.icon();
        } catch (...) {}
        attrs["description"] = metadata.description();
        attrs["uri"] = scopes::CannedQuery(metadata.scope_id()).to_uri();
        map["attrs"] = attrs;

        return map;
    }
};

OverviewResultsModel::OverviewResultsModel(QObject* parent)
 : unity::shell::scopes::ResultsModelInterface(parent)
{
}

void OverviewResultsModel::setResults(const QList<unity::scopes::ScopeMetadata::SPtr>& results)
{
    if (m_results.empty()) {
        beginResetModel();
        m_results = results;
        endResetModel();
        Q_EMIT countChanged();
        return;
    }

    int pos = 0;
    QMap<QString, int> newResult;
    for (auto const res: results) {
        newResult[QString::fromStdString(res->scope_id())] = pos++;
    }

    // itearate over old results, remove rows that are not present in new results
    int row = 0;
    for (auto it = m_results.begin(); it != m_results.end();)
    {
        if (!newResult.contains(QString::fromStdString((*it)->scope_id()))) {
            beginRemoveRows(QModelIndex(), row, row);
            it = m_results.erase(it);
            endRemoveRows();
        } else {
            ++it;
            ++row;
        }
    }

    QSet<QString> oldResult;
    for (auto const res: m_results) {
        oldResult.insert(QString::fromStdString(res->scope_id()));
    }

    // iterate over new results, insert rows if not in previous model
    row = 0;
    for (auto const newRes: results)
    {
        if (!oldResult.contains(QString::fromStdString(newRes->scope_id())))
        {
            beginInsertRows(QModelIndex(), row, row);
            m_results.insert(row, newRes);
            endInsertRows();
        }
        ++row;
    }

    // iterate over results, move rows if positions changes
    for (int i = 0; i<m_results.size(); )
    {
        auto scope_meta = m_results.at(i);
        const QString id = QString::fromStdString(scope_meta->scope_id());
        if (newResult.contains(id)) {
            pos = newResult[id];
            if (pos != i) {
                beginMoveRows(QModelIndex(), i, i, QModelIndex(), pos);
                m_results.move(i, pos);
                endMoveRows();
                continue;
            }
        }
        i++;
    }

    Q_EMIT countChanged();
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
    scopes::ScopeMetadata* metadata = m_results.at(index.row()).get();

    switch (role) {
        case RoleUri: {
            scopes::CannedQuery q(metadata->scope_id());
            return QString::fromStdString(q.to_uri());
        }
        case RoleCategoryId:
            return QVariant();
        case RoleDndUri:
            return data(index, RoleUri);
        case RoleResult: {
            scopes::Result::SPtr result(new FakeResult(*metadata));
            return QVariant::fromValue(result);
        }
        case RoleTitle:
            return QString::fromStdString(metadata->display_name());
        case RoleArt: {
            std::string art;
            try {
                art = metadata->icon();
            } catch (...) {
                try {
                    art = metadata->art();
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
        case RoleOverlayColor: {
            try {
                std::string color;
                auto attrs = metadata->appearance_attributes();
                auto it = attrs.find("logo-overlay-color");
                if (it != attrs.end()) {
                    color = it->second.get_string();
                    return QString::fromStdString(color);
                }
            } catch (...) {
                // silently ignore
            }
            return QVariant();
        }
        case RoleScopeId:
            return QString::fromStdString(metadata->scope_id());
        default:
            return QVariant();
    }
}

} // namespace scopes_ng
