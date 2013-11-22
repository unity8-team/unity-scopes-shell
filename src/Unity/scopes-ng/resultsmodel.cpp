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
    m_roles[ResultsModel::RoleIconHint] = "icon";
    m_roles[ResultsModel::RoleCategory] = "category";
    m_roles[ResultsModel::RoleTitle] = "title";
    m_roles[ResultsModel::RoleComment] = "comment";
    m_roles[ResultsModel::RoleDndUri] = "dndUri";
    m_roles[ResultsModel::RoleMetadata] = "metadata";
    m_roles[ResultsModel::RoleRendererHints] = "rendererHints";
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

void ResultsModel::addResults(QList<std::shared_ptr<unity::api::scopes::ResultItem>> const& results)
{
    if (results.count() == 0) return;
    
    beginInsertRows(QModelIndex(), m_results.count(), m_results.count() + results.count() - 1);
    Q_FOREACH(std::shared_ptr<scopes::ResultItem> const& result, results) {
        m_results.append(result);
    }
    endInsertRows();
}

void ResultsModel::clearResults()
{
    if (m_results.count() == 0) return;

    beginRemoveRows(QModelIndex(), 0, m_results.count() - 1);
    m_results.clear();
    endRemoveRows();
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

QVariant
ResultsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_results.count()) {
        return QVariant();
    }

    scopes::ResultItem* result = m_results[index.row()].get();

    switch (role) {
        case RoleUri:
            return QVariant(QString::fromStdString(result->uri()));
        case RoleIconHint: {
            QString iconHint(QString::fromStdString(result->icon()));
            if (iconHint.isEmpty()) {
                QString uri(QString::fromStdString(result->uri()));
                QString thumbnailerUri(uriToThumbnailerProviderString(uri, QString(), QVariantHash()));
                if (!thumbnailerUri.isNull()) {
                    return QVariant::fromValue(thumbnailerUri);
                }
            }
            return QVariant(iconHint);
        }
        case RoleCategory:
            return QVariant(QString::fromStdString(result->category()->id()));
        case RoleTitle:
            return QVariant(QString::fromStdString(result->title()));
        case RoleComment:
            return QVariant();
        case RoleDndUri:
            return QVariant(QString::fromStdString(result->dnd_uri()));
        case RoleMetadata:
            return QVariant(QVariantMap());
        case RoleRendererHints:
            return QVariant();
        default:
            return QVariant();
    }
}

} // namespace scopes_ng
