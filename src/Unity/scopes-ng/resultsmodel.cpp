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

#include "../iconutils.h"

namespace scopes_ng {

using namespace unity::api;

ResultsModel::ResultsModel(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roles[ResultsModel::RoleUri] = "uri";
    m_roles[ResultsModel::RoleCategoryId] = "categoryId";
    m_roles[ResultsModel::RoleDndUri] = "dndUri";
    m_roles[ResultsModel::RoleMetadata] = "metadata";
    m_roles[ResultsModel::RoleTitle] = "title";
    m_roles[ResultsModel::RoleArt] = "art";
    m_roles[ResultsModel::RoleSubtitle] = "subtitle";
    m_roles[ResultsModel::RoleMascot] = "mascot";
    m_roles[ResultsModel::RoleEmblem] = "emblem";
    m_roles[ResultsModel::RoleOldPrice] = "oldPrice";
    m_roles[ResultsModel::RolePrice] = "price";
    m_roles[ResultsModel::RoleAltPrice] = "altPrice";
    m_roles[ResultsModel::RoleRating] = "rating";
    m_roles[ResultsModel::RoleAltRating] = "altRating";
    m_roles[ResultsModel::RoleSummary] = "summary";
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

void ResultsModel::addResults(QList<std::shared_ptr<unity::api::scopes::CategorisedResult>> const& results)
{
    if (results.count() == 0) return;
    
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

QHash<int, QByteArray>
ResultsModel::roleNames() const
{
    return m_roles;
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
ResultsModel::componentValue(scopes::CategorisedResult* result, std::string const& fieldName) const
{
    // FIXME: component field mapping
    if (!result->has_metadata(fieldName)) {
        return QVariant();
    }
    scopes::Variant const& v = result->metadata(fieldName);
    if (v.which() != scopes::Variant::Type::String) {
        return QVariant();
    }

    return QString::fromStdString(v.get_string());
}

QVariant
ResultsModel::data(const QModelIndex& index, int role) const
{
    scopes::CategorisedResult* result = m_results.at(index.row()).get();

    switch (role) {
        case RoleUri:
            return QString::fromStdString(result->uri());
        case RoleCategoryId:
            return QString::fromStdString(result->category()->id());
        case RoleDndUri:
            return QString::fromStdString(result->dnd_uri());
        case RoleMetadata:
            return QVariantMap(); // FIXME! would be great to keep it opaque, so it isn't misused
        case RoleTitle:
            return QString::fromStdString(result->title());
        case RoleArt: {
            QString image(QString::fromStdString(result->art()));
            if (image.isEmpty()) {
                QString uri(QString::fromStdString(result->uri()));
                // FIXME: what to do about mimetype?
                QString thumbnailerUri(uriToThumbnailerProviderString(uri, QString(), QVariantHash()));
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
        case RoleOldPrice:
            return componentValue(result, "old-price");
        case RolePrice:
            return componentValue(result, "price");
        case RoleAltPrice:
            return componentValue(result, "alt-price");
        case RoleRating:
            return componentValue(result, "rating");
        case RoleAltRating:
            return componentValue(result, "alt-rating");
        case RoleSummary:
            return componentValue(result, "summary");
        default:
            return QVariant();
    }
}

} // namespace scopes_ng
