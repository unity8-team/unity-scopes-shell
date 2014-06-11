/*
 * setttingsmodel.cpp
 *
 *  Created on: 10 Jun 2014
 *      Author: pete
 */

#include "settingsmodel.h"

#include <QDebug>
#include <QDir>
#include <QJsonDocument>

using namespace scopes_ng;

static const QString SETTING_GROUP("default");

static const QString SETTING_ID_PATTERN("%1-%2");

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
        QVariantMap parameters = data["parameters"].toMap();
        QString id = data["id"].toString();

        QSharedPointer<U1db::Document> document(new U1db::Document);
        document->setDatabase(&m_database);
        document->setDocId(SETTING_ID_PATTERN.arg(SETTING_GROUP, id));
        QVariantMap defaults;
        defaults["value"] = parameters["defaultValue"];
        document->setDefaults(defaults);
        document->setCreate(true);

        m_documents[id] = document;

        QVariantMap contents = document->getContents().toMap();
        parameters["currentValue"] = contents["value"];

        QSharedPointer<Data> setting(
                new Data(id, data["displayName"].toString(),
                        data["type"].toString(), parameters));

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
            default:
                break;
        }
    }

    return result;
}

int SettingsModel::rowCount(const QModelIndex&) const
{
    return m_data.size();
}

void SettingsModel::setValue(const QString& settingName, const QVariant& value)
{
    QSharedPointer<U1db::Document> document = m_documents[settingName];
    if (!document)
    {
        return;
    }

    QVariantMap map;
    map["value"] = value;
    document->setContents(map);
}
