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
#include "utils.h"

#include <QDebug>
#include <QDir>
#include <QTimer>

using namespace scopes_ng;
namespace sc = unity::scopes;

static const QString SETTING_GROUP("default");

static const QString SETTING_ID_PATTERN("%1-%2");

SettingsModel::SettingsModel(const QDir& shareDir, const QString& scopeId,
        const QVariant& settingsDefinitions, QObject* parent,
        int settingsTimeout)
        : SettingsModelInterface(parent), m_settingsTimeout(settingsTimeout)
{
    shareDir.mkdir(scopeId);
    QDir databaseDir = shareDir.filePath(scopeId);
    m_database.setPath(databaseDir.filePath("settings.db"));

    for (const auto &it : settingsDefinitions.toList())
    {
        QVariantMap data = it.toMap();
        QString id = data["id"].toString();
        QString displayName = data["displayName"].toString();
        QVariantMap properties = data["parameters"].toMap();
        QString type = data["type"].toString();

        QVariantMap defaults;
        defaults["value"] = properties["defaultValue"];

        QSharedPointer<U1db::Document> document(new U1db::Document);
        document->setDatabase(&m_database);
        document->setDocId(SETTING_ID_PATTERN.arg(SETTING_GROUP, id));
        document->setDefaults(defaults);
        document->setCreate(true);

        m_documents[id] = document;

        QSharedPointer<QTimer> timer(new QTimer());
        timer->setProperty("setting_id", id);
        timer->setSingleShot(true);
        timer->setInterval(m_settingsTimeout);
        timer->setTimerType(Qt::VeryCoarseTimer);
        connect(timer.data(), SIGNAL(timeout()), this,
                SLOT(settings_timeout()));
        m_timers[id] = timer;

        QSharedPointer<Data> setting(
                new Data(id, displayName, type, properties));

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
                result = data->properties;
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
                QSharedPointer<QTimer> timer = m_timers[data->id];
                timer->setProperty("value", value);
                timer->start();

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
    return count();
}

int SettingsModel::count() const
{
    return m_data.size();
}

void SettingsModel::settings_timeout()
{
    QObject *timer = sender();
    if (!timer)
    {
        return;
    }

    QString setting_id = timer->property("setting_id").toString();
    QVariant value = timer->property("value");

    QSharedPointer<U1db::Document> document = m_documents[setting_id];
    QVariantMap map;
    map["value"] = value;
    document->setContents(map);
}
