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

// self
#include "resultsmodel.h"

// local
#include "utils.h"
#include "iconutils.h"
#include "resultsmap.h"

#include <map>
#include <QDebug>

namespace scopes_ng {

using namespace unity;

ResultsModel::ResultsModel(QObject* parent)
 : unity::shell::scopes::ResultsModelInterface(parent)
 , m_maxAttributes(2)
 , m_purge(true)
{
}

QString ResultsModel::categoryId() const
{
    return m_categoryId;
}

void ResultsModel::setCategoryId(QString const& id)
{
    if (m_categoryId != id) {
        m_categoryId = id;
        Q_EMIT categoryIdChanged();
    }
}

void ResultsModel::setComponentsMapping(QHash<QString, QString> const& mapping)
{
    std::unordered_map<std::string, std::string> newMapping;
    for (auto it = mapping.begin(); it != mapping.end(); ++it) {
        newMapping[it.key().toStdString()] = it.value().toStdString();
    }

    if (rowCount() > 0) {
        beginResetModel();
        m_componentMapping.swap(newMapping);
        endResetModel();
    } else {
        m_componentMapping.swap(newMapping);
    }
}

void ResultsModel::setMaxAtrributesCount(int count)
{
    m_maxAttributes = count;
}

void ResultsModel::addUpdateResults(QList<std::shared_ptr<unity::scopes::CategorisedResult>>& results)
{
    if (results.count() == 0) {
        return;
    }

    m_purge = false;

    const int oldCount = m_results.count();

    ResultsMap newResultsMap(results);

    int row = 0;
    // iterate over old (i.e. currently visible) results, remove results which are no longer present in new set
    for (auto it = m_results.begin(); it != m_results.end(); ) {
        int newPos = newResultsMap.find(*it);
        bool haveNow = (newPos >= 0);
        if (!haveNow) {
            // delete row
            beginRemoveRows(QModelIndex(), row, row);
            it = m_results.erase(it);
            endRemoveRows();
        } else {
            ++it;
            ++row;
        }
    }

    ResultsMap oldResultsMap(m_results);

    // iterate over new results
    for (row = 0; row<results.count(); ++row) {
        const int oldPos = oldResultsMap.find(results[row]);
        const bool hadBefore = (oldPos >= 0);
        if (hadBefore) {
            if (row != oldPos) {
                // move row
                beginMoveRows(QModelIndex(), oldPos, oldPos, QModelIndex(), row + (row > oldPos ? 1 : 0));
                m_results.move(oldPos, row);
                oldResultsMap.rebuild(m_results);
                endMoveRows();
            }
        } else {
            // insert row
            beginInsertRows(QModelIndex(), row, row);
            m_results.insert(row, results[row]);
            oldResultsMap.rebuild(m_results);
            endInsertRows();
        }
    }

    if (oldCount != m_results.count()) {
        Q_EMIT countChanged();
    }
}

void ResultsModel::addResults(QList<std::shared_ptr<unity::scopes::CategorisedResult>>& results)
{
    if (results.count() == 0) {
        return;
    }

    m_purge = false;

    ResultsMap newResultsMap(results); // deduplicate results
    Q_UNUSED(newResultsMap);

    beginInsertRows(QModelIndex(), m_results.count(), m_results.count() + results.count() - 1);
    Q_FOREACH(std::shared_ptr<scopes::CategorisedResult> const& result, results) {
        m_results.append(result);
    }
    endInsertRows();

    Q_EMIT countChanged();
}

void ResultsModel::clearResults()
{
    if (m_results.count() == 0) return;

    beginRemoveRows(QModelIndex(), 0, m_results.count() - 1);
    m_results.clear();
    endRemoveRows();

    Q_EMIT countChanged();
}

int ResultsModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return m_results.count();
}

int ResultsModel::count() const
{
    return m_results.count();
}

QVariant
ResultsModel::componentValue(scopes::Result const* result, std::string const& fieldName) const
{
    auto mappingIt = m_componentMapping.find(fieldName);
    if (mappingIt == m_componentMapping.end()) {
        return QVariant();
    }
    std::string const& realFieldName = mappingIt->second;
    try {
        scopes::Variant const& v = result->value(realFieldName);
        return scopeVariantToQVariant(v);
    } catch (...) {
        // value() throws if realFieldName is empty or the result
        // doesn't have a value for it
        return QVariant();
    }
}

QVariant
ResultsModel::attributesValue(scopes::Result const* result) const
{
    auto mappingIt = m_componentMapping.find("attributes");
    if (mappingIt == m_componentMapping.end()) {
        return QVariant();
    }
    try {
        std::string const& realFieldName = mappingIt->second;
        scopes::Variant const& v = result->value(realFieldName);
        if (v.which() != scopes::Variant::Type::Array) {
            return QVariant();
        }

        QVariantList attributes;
        scopes::VariantArray arr(v.get_array());
        for (size_t i = 0; i < arr.size(); i++) {
            if (arr[i].which() != scopes::Variant::Type::Dict) {
                continue;
            }
            QVariantMap attribute(scopeVariantToQVariant(arr[i]).toMap());
            attributes << QVariant(attribute);
            // we'll limit the number of attributes
            if (attributes.size() >= m_maxAttributes) {
                break;
            }
        }

        return attributes;
    } catch (...) {
        // value() throws if realFieldName is empty or the result
        // doesn't have a value for it
        return QVariant();
    }
}

QHash<int, QByteArray> ResultsModel::roleNames() const
{
    QHash<int, QByteArray> roles(unity::shell::scopes::ResultsModelInterface::roleNames());
    roles.insert(ExtraRoles::RoleScopeId, "scopeId");

    return roles;
}

void ResultsModel::updateResult(scopes::Result const& result, scopes::Result const& updatedResult)
{
    for (int i = 0; i<m_results.size(); i++)
    {
        auto const res = m_results[i];
        if (result.uri() == res->uri() && result.serialize() == res->serialize())
        {
            qDebug() << "Updated result with uri '" << QString::fromStdString(res->uri()) << "'";
            m_results[i] = std::make_shared<scopes::Result>(updatedResult);
            auto const idx = index(i, 0);
            Q_EMIT dataChanged(idx, idx);
            return;
        }
    }
    qWarning() << "ResultsModel::updateResult - failed to find result with uri '"
        << QString::fromStdString(result.uri())
        << "', category '" << categoryId() << "'";
}

QVariant
ResultsModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if (row >= m_results.size())
    {
        qWarning() << "ResultsModel::data - invalid index" << row << "size"
                << m_results.size();
        return QVariant();
    }

    scopes::Result* result = m_results.at(index.row()).get();

    switch (role) {
        case RoleUri:
            return QString::fromStdString(result->uri());
        case RoleCategoryId:
            return categoryId();
        case RoleDndUri:
            return QString::fromStdString(result->dnd_uri());
        case RoleResult:
            return QVariant::fromValue(std::static_pointer_cast<unity::scopes::Result>(m_results.at(index.row())));
        case RoleTitle:
            return componentValue(result, "title");
        case RoleArt: {
            QString image(componentValue(result, "art").toString());
            if (image.isEmpty()) {
                QString uri(QString::fromStdString(result->uri()));
                // FIXME: figure out a better way and get rid of this, it's an awful hack
                QVariantHash result_meta;
                if (result->contains("artist") && result->contains("album")) {
                    result_meta[QStringLiteral("artist")] = scopeVariantToQVariant(result->value("artist"));
                    result_meta[QStringLiteral("album")] = scopeVariantToQVariant(result->value("album"));
                }
                QString thumbnailerUri(uriToThumbnailerProviderString(uri, result_meta));
                if (!thumbnailerUri.isNull()) {
                    return thumbnailerUri;
                }
            }
            return image;
        }
        case RoleSubtitle:
            return componentValue(result, "subtitle");
        case RoleMascot:
            return componentValue(result, "mascot");
        case RoleEmblem:
            return componentValue(result, "emblem");
        case RoleAttributes:
            return attributesValue(result);
        case RoleSummary:
            return componentValue(result, "summary");
        case RoleBackground: {
            QVariant backgroundVariant(componentValue(result, "background"));
            if (backgroundVariant.isNull()) {
                return backgroundVariant;
            }
            return backgroundUriToVariant(backgroundVariant.toString());
        }
        case RoleOverlayColor:
            return componentValue(result, "overlay-color");
        case RoleScopeId:
            if (result->uri().compare(0, 8, "scope://") == 0) {
                try {
                    scopes::CannedQuery q(scopes::CannedQuery::from_uri(result->uri()));
                    return QString::fromStdString(q.scope_id());
                } catch (...) {
                    // silently ignore and return "undefined"
                }
            }
            return QVariant();
        case RoleQuickPreviewData:
            return componentValue(result, "quick-preview-data");
        case RoleSocialAttributes:
            return componentValue(result, "social-attributes");
        default:
            return QVariant();
    }
}

void ResultsModel::markNewSearch()
{
    m_purge = true;
}

bool ResultsModel::needsPurging() const
{
    return m_purge;
}

} // namespace scopes_ng
