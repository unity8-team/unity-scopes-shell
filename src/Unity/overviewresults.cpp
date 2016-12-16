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
#include <unity/scopes/Scope.h>
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

void OverviewResultsModel::setResults(const QList<unity::scopes::ScopeMetadata::SPtr>& results, const QMap<QString, QString>& scopeIdToName)
{
    if (m_results.empty()) {
        beginResetModel();
        m_results = results;
        for (auto const newRes: results)
        {
            updateChildScopes(newRes, scopeIdToName);
        }

        endResetModel();
        Q_EMIT countChanged();
        return;
    }

    int pos = 0;
    QMap<QString, int> newResult;
    for (auto const res: results) {
        newResult[QString::fromStdString(res->scope_id())] = pos++;
    }

    // iterate over old results, remove rows that are not present in new results
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
        if (updateChildScopes(newRes, scopeIdToName))
        {
            // update aggregator subtitles when child scopes change
            Q_EMIT dataChanged(index(row), index(row), {RoleSubtitle});
        }
        if (!oldResult.contains(QString::fromStdString(newRes->scope_id())))
        {
            beginInsertRows(QModelIndex(), row, row);
            m_results.insert(row, newRes);
            endInsertRows();
        }
        ++row;
    }


    // Iterate over results, move rows if positions changes.
    // Use the ordering of newResults to sort m_results.
    // Since the list is small and usually already sorted, or has only a
    // single element out of order, insertion sort suffices.
    QList<int> sort_keys;
    for (int i = 0; i < m_results.size(); i++){
        QString current_string = QString::fromStdString(m_results.at(i)->scope_id());
        sort_keys.append(newResult[current_string]);
    }

    for (int i = 1; i < m_results.size(); i++) {
        int j = i;
        while (j > 0 && sort_keys.at(j - 1) > sort_keys.at(j)) {
            sort_keys.swap(j, j - 1);
            beginMoveRows(QModelIndex(), j, j, QModelIndex(), j - 1);
            m_results.move(j - 1, j);
            endMoveRows();
            j--;
        }
    }

    Q_EMIT countChanged();
}

bool OverviewResultsModel::updateChildScopes(const unity::scopes::ScopeMetadata::SPtr& scopeMetadata, const QMap<QString, QString>& scopeIdToName)
{
    if (!scopeMetadata->is_aggregator())
    {
        ///!===
        /// TODO: This code should be removed as soon as we can remove child_scope_ids() from ScopeMetadata.
        /// Aggregators should now be implementing the find_child_scopes() method rather than setting ChildScopes in config.
        auto const children = scopeMetadata->child_scope_ids();
        if (children.size())
        {
            // iterate over child scope ids, join their display names and insert into m_childScopes for current scope
            QStringList childNames;
            for (auto const& id: children)
            {
                auto it = scopeIdToName.find(QString::fromStdString(id));
                if (it != scopeIdToName.end())
                {
                    childNames << *it;
                }
            }
            if (!childNames.empty())
            {
                m_childScopes[QString::fromStdString(scopeMetadata->scope_id())] = childNames.join(QStringLiteral(", "));
            }
        }
        ///!===
        return false;
    }

    unity::scopes::ChildScopeList children;
    try
    {
        children = scopeMetadata->proxy()->child_scopes();
    }
    catch (std::exception const& e)
    {
        qWarning("OverviewResultsModel::updateChildScopes: Exception caught from proxy()->child_scopes(): %s", e.what());
        return false;
    }

    if (children.size())
    {
        // iterate over child scope ids, join their display names and insert into m_childScopes for current scope
        QStringList childNames;
        for (auto const& child : children)
        {
            auto it = scopeIdToName.find(QString::fromStdString(child.id));
            if (it != scopeIdToName.end())
            {
                childNames << *it;
            }
        }
        if (!childNames.empty())
        {
            m_childScopes[QString::fromStdString(scopeMetadata->scope_id())] = childNames.join(QStringLiteral(", "));
        }
    }
    else
    {
        m_childScopes[QString::fromStdString(scopeMetadata->scope_id())] = QLatin1String("");
    }
    return true;
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
        case RoleSubtitle: {
            auto it = m_childScopes.find(QString::fromStdString(metadata->scope_id()));
            if (it != m_childScopes.end())
            {
                return *it;
            }
            return QVariant();
        }
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
