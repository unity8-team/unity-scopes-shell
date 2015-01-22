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
#include <QTextCodec>
#include <QTimer>

using namespace scopes_ng;
namespace sc = unity::scopes;

SettingsModel::SettingsModel(const QDir& configDir, const QString& scopeId,
        const QVariant& settingsDefinitions, QObject* parent,
        int settingsTimeout)
        : SettingsModelInterface(parent), m_scopeId(scopeId), m_settingsTimeout(settingsTimeout)
{
    configDir.mkpath(scopeId);
    QDir databaseDir = configDir.filePath(scopeId);

    m_settings.reset(new QSettings(databaseDir.filePath("settings.ini"), QSettings::IniFormat));
    m_settings->setIniCodec("UTF-8");

    for (const auto &it : settingsDefinitions.toList())
    {
        QVariantMap data = it.toMap();
        QString id = data["id"].toString();
        QString displayName = data["displayName"].toString();
        QVariantMap properties;
        QVariant defaultValue;
        if (data.contains("displayValues"))
        {
            properties["values"] = data["displayValues"].toList();
        }
        QString type = data["type"].toString();

        QVariant::Type variantType = QVariant::Invalid;

        if(type == "boolean")
        {
            variantType = QVariant::Bool;
        }
        else if(type == "list")
        {
            variantType = QVariant::UInt;
        }
        else if(type == "number")
        {
            variantType = QVariant::Double;
        }
        else if(type == "string")
        {
            variantType = QVariant::String;
        }

        if(data.contains("defaultValue"))
        {
            defaultValue = data["defaultValue"];
            properties["defaultValue"] = defaultValue;
        }

        QSharedPointer<QTimer> timer(new QTimer());
        timer->setProperty("setting_id", id);
        timer->setSingleShot(true);
        timer->setInterval(m_settingsTimeout);
        timer->setTimerType(Qt::VeryCoarseTimer);
        connect(timer.data(), SIGNAL(timeout()), this,
                SLOT(settings_timeout()));
        m_timers[id] = timer;

        QSharedPointer<Data> setting(
                new Data(id, displayName, type, properties, defaultValue,
                        variantType));

        m_data << setting;
        m_data_by_id[id] = setting;
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
                result = m_settings->value(data->id, data->defaultValue);
                result.convert(data->variantType);
                break;
            }
            default:
                break;
        }
    }
    else if (row - m_data.size() < m_child_scopes_data.size())
    {
        auto data = m_child_scopes_data[row - m_data.size()];

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
                result = data->defaultValue;
                break;
            default:
                break;
        }
    }

    return result;
}

QVariant SettingsModel::value(const QString& id) const
{
    m_settings->sync();

    QVariant result;

    if (m_child_scopes_data_by_id.contains(id))
    {
        result = m_child_scopes_data_by_id[id]->defaultValue;
    }
    else if (m_data_by_id.contains(id))
    {
        QSharedPointer<Data> data = m_data_by_id[id];
        result = m_settings->value(data->id, data->defaultValue);
        result.convert(data->variantType);
    }

    return result;
}

void SettingsModel::update_child_scopes(QMap<QString, sc::ScopeMetadata::SPtr> const& scopes_metadata)
{
    if (!scopes_metadata.contains(m_scopeId) ||
        !scopes_metadata[m_scopeId]->is_aggregator())
    {
        return;
    }

    auto scope_proxy = scopes_metadata[m_scopeId]->proxy();
    try
    {
        scope_proxy->set_child_scopes_ordered(m_child_scopes);
        m_child_scopes = scope_proxy->child_scopes_ordered();
    }
    catch (std::exception const& e)
    {
        return;
    }

    m_child_scopes_data.clear();
    m_child_scopes_data_by_id.clear();
    m_child_scopes_timers.clear();

    for (sc::ChildScope const& child_scope : m_child_scopes)
    {
        QString id = child_scope.id.c_str();
        QString displayName = scopes_metadata[id]->display_name().c_str();
        QVariant defaultValue = child_scope.enabled;
        QVariantMap properties;
        properties["defaultValue"] = defaultValue;

        QSharedPointer<QTimer> timer(new QTimer());
        timer->setProperty("setting_id", id);
        timer->setSingleShot(true);
        timer->setInterval(m_settingsTimeout);
        timer->setTimerType(Qt::VeryCoarseTimer);
        connect(timer.data(), SIGNAL(timeout()), this,
                SLOT(settings_timeout()));
        m_child_scopes_timers[id] = timer;

        QSharedPointer<Data> setting(
                new Data(id, displayName, "boolean", properties, defaultValue,
                        QVariant::Bool));

        m_child_scopes_data << setting;
        m_child_scopes_data_by_id[id] = setting;
    }
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
    else if (row - m_data.size() < m_child_scopes_data.size())
    {
        auto data = m_child_scopes_data[row - m_data.size()];

        switch (role)
        {
            case Roles::RoleValue:
            {
                QSharedPointer<QTimer> timer = m_child_scopes_timers[data->id];
                timer->setProperty("index", row - m_data.size());
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
    return m_data.size() + m_child_scopes_data.size();
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

    if (m_child_scopes_data_by_id.contains(setting_id))
    {
        int setting_index = timer->property("index").toInt();
        std::list<sc::ChildScope>::iterator it = std::next(m_child_scopes.begin(), setting_index);
        it->enabled = value.toBool();
    }
    else if (m_data_by_id.contains(setting_id))
    {
        m_settings->setValue(setting_id, value);
    }

    Q_EMIT settingsChanged();
}
