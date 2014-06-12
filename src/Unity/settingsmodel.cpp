/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *  Pete Woods <pete.woods@canonical.com>
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

#include "settingsmodel.h"

#include <QDebug>
#include <QDir>
#include <QJsonDocument>

using namespace scopes_ng;

static const QString SETTING_GROUP("default");

static const QString SETTING_ID_PATTERN("%1-%2");

static const QSet<QString> VALID_TYPES { "boolean", "list", "number", "string" };

SettingsModel::SettingsModel(const QString& scopeId, const QByteArray& json,
        QObject* parent)
        : SettingsModelInterface(parent)
{
    QDir shareDir = QDir::home().filePath(".local/share");
    shareDir.mkdir(scopeId);
    QDir databaseDir = shareDir.filePath(scopeId);
    m_database.setPath(databaseDir.filePath("settings.db"));

    QJsonDocument jsonDocument = QJsonDocument::fromJson(json);
    QVariant variant = jsonDocument.toVariant();
    QVariantList settings = variant.toMap()["settings"].toList();

    for (const QVariant &variant : settings)
    {
        QVariantMap data = variant.toMap();

        QString id = data["id"].toString();
        QString displayName = data["displayName"].toString();
        QString type = data["type"].toString();
        QVariantMap parameters = data["parameters"].toMap();

        if (id.isEmpty() || displayName.isEmpty())
        {
            continue;
        }
        if (!VALID_TYPES.contains(type))
        {
            continue;
        }

        QVariantMap defaults;
        defaults["value"] = parameters["defaultValue"];

        QSharedPointer<U1db::Document> document(new U1db::Document);
        document->setDatabase(&m_database);
        document->setDocId(SETTING_ID_PATTERN.arg(SETTING_GROUP, id));
        document->setDefaults(defaults);
        document->setCreate(true);

        m_documents[id] = document;

        QSharedPointer<Data> setting(
                new Data(id, displayName, type, parameters));

        m_data << setting;
    }
}

QVariant SettingsModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    QVariant result;

    if (row < m_data.size())
    {
        auto data = m_data[row];

        switch (role)
        {
            case Roles::RoleSettingId:
                result = data->id;
                break;
            case Roles::RoleDisplayName:
                result = data->displayName;
                break;
            case Roles::RoleType:
                result = data->type;
                break;
            case Roles::RoleProperties:
                result = data->data;
                break;
            case Roles::RoleValue:
            {
                QSharedPointer<U1db::Document> document = m_documents[data->id];
                QVariantMap contents = document->getContents().toMap();
                result = contents["value"];
                break;
            }
            default:
                break;
        }
    }

    return result;
}

bool SettingsModel::setData(const QModelIndex &index, const QVariant &value,
        int role)
{
    int row = index.row();
    QVariant result;

    if (row < m_data.size())
    {
        auto data = m_data[row];

        switch (role)
        {
            case Roles::RoleValue:
            {
                QSharedPointer<U1db::Document> document = m_documents[data->id];
                QVariantMap map;
                map["value"] = value;
                document->setContents(map);

                return true;
            }
            default:
                break;
        }
    }

    return false;
}

int SettingsModel::rowCount(const QModelIndex&) const
{
    return m_data.size();
}
