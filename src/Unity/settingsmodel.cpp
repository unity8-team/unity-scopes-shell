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
#include "localization.h"
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
        : SettingsModelInterface(parent), m_scopeId(scopeId), m_settingsTimeout(settingsTimeout),
          m_requireChildScopesRefresh(false)
{
    configDir.mkpath(scopeId);
    QDir databaseDir = configDir.filePath(scopeId);

    m_settings.reset(new QSettings(databaseDir.filePath(QStringLiteral("settings.ini")), QSettings::IniFormat));
    m_settings->setIniCodec("UTF-8");

    for (const auto &it : settingsDefinitions.toList())
    {
        QVariantMap data = it.toMap();
        QString id = data[QStringLiteral("id")].toString();
        QString displayName = data[QStringLiteral("displayName")].toString();
        QVariantMap properties;
        QVariant defaultValue;
        if (data.contains(QStringLiteral("displayValues")))
        {
            properties[QStringLiteral("values")] = data[QStringLiteral("displayValues")].toList();
        }
        QString type = data[QStringLiteral("type")].toString();

        QVariant::Type variantType = QVariant::Invalid;

        if(type == QLatin1String("boolean"))
        {
            variantType = QVariant::Bool;
        }
        else if(type == QLatin1String("list"))
        {
            variantType = QVariant::UInt;
        }
        else if(type == QLatin1String("number"))
        {
            variantType = QVariant::Double;
        }
        else if(type == QLatin1String("string"))
        {
            variantType = QVariant::String;
        }

        if(data.contains(QStringLiteral("defaultValue")))
        {
            defaultValue = data[QStringLiteral("defaultValue")];
            properties[QStringLiteral("defaultValue")] = defaultValue;
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
    QMutexLocker locker(&m_mutex);

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
            {
                auto it = std::next(m_child_scopes.begin(), row - m_data.size());
                result = it->enabled;
                break;
            }
            default:
                break;
        }
    }

    return result;
}

QVariant SettingsModel::value(const QString& id) const
{
    QMutexLocker locker(&m_mutex);

    m_settings->sync();

    // Check for the setting id in the child scopes list first, in case the
    // aggregator is incorrectly using a scope id as a settings as well.
    if (m_child_scopes_data_by_id.contains(id))
    {
        for (auto const& child_scope : m_child_scopes)
        {
            if (child_scope.id == id.toStdString())
            {
                return child_scope.enabled;
            }
        }
    }
    else if (m_data_by_id.contains(id))
    {
        QSharedPointer<Data> data = m_data_by_id[id];
        auto result = m_settings->value(data->id, data->defaultValue);
        result.convert(data->variantType);
        return result;
    }

    return QVariant();
}

void SettingsModel::update_child_scopes(QMap<QString, sc::ScopeMetadata::SPtr> const& scopes_metadata)
{
    QMutexLocker locker(&m_mutex);

    if (!scopes_metadata.contains(m_scopeId) ||
        !scopes_metadata[m_scopeId]->is_aggregator())
    {
        return;
    }

    m_scopeProxy = scopes_metadata[m_scopeId]->proxy();
    try
    {
        m_child_scopes = m_scopeProxy->child_scopes();
    }
    catch (std::exception const& e)
    {
        qWarning("SettingsModel::update_child_scopes: Exception caught from m_scopeProxy->child_scopes_ordered(): %s", e.what());
        return;
    }

    const bool reset = m_requireChildScopesRefresh;
    if (reset) {
        // Reset the settings model to fix LP: #1484299, where a new child scope just finished installing
        // while settings view is created (and we crash); since this is really a corner case, just
        // resetting the model is fine.
        beginResetModel();
    }

    m_requireChildScopesRefresh = false;

    m_child_scopes_data.clear();
    m_child_scopes_data_by_id.clear();
    m_child_scopes_timers.clear();

    for (sc::ChildScope const& child_scope : m_child_scopes)
    {
        QString id = child_scope.id.c_str();
        if (!scopes_metadata.contains(id)) {
            // if a child scope was just added to the registry, then scopes_metadata may not contain it yet because of the
            // scope registry refresh delay on scope add/removal (see LP: #1484299).
            qWarning() << "SettingsModel::update_child_scopes(): no scope with id '" + id + "'";
            m_requireChildScopesRefresh = true;
            continue;
        }
        QString displayName = QString::fromStdString(_("Display results from %1")).arg(QString(scopes_metadata[id]->display_name().c_str()));

        QSharedPointer<QTimer> timer(new QTimer());
        timer->setProperty("setting_id", id);
        timer->setSingleShot(true);
        timer->setInterval(m_settingsTimeout);
        timer->setTimerType(Qt::VeryCoarseTimer);
        connect(timer.data(), SIGNAL(timeout()), this,
                SLOT(settings_timeout()));
        m_child_scopes_timers[id] = timer;

        QSharedPointer<Data> setting(
                new Data(id, displayName, QStringLiteral("boolean"), QVariantMap(), QVariant(), QVariant::Bool));

        m_child_scopes_data << setting;
        m_child_scopes_data_by_id[id] = setting;
    }

    if (reset) {
        endResetModel();
        locker.unlock();
        Q_EMIT countChanged();
    }
}

bool SettingsModel::setData(const QModelIndex &index, const QVariant &value,
        int role)
{
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);
    return m_data.size() + m_child_scopes_data.size();
}

bool SettingsModel::require_child_scopes_refresh() const
{
    QMutexLocker locker(&m_mutex);
    return m_requireChildScopesRefresh;
}

void SettingsModel::settings_timeout()
{
    QMutexLocker locker(&m_mutex);

    QObject *timer = sender();
    if (!timer)
    {
        return;
    }

    QString setting_id = timer->property("setting_id").toString();
    QVariant value = timer->property("value");

    // Check for the setting id in the child scopes list first, in case the
    // aggregator is incorrectly using a scope id as a settings as well.
    if (m_child_scopes_data_by_id.contains(setting_id))
    {
        int setting_index = timer->property("index").toInt();
        auto it = std::next(m_child_scopes.begin(), setting_index);
        it->enabled = value.toBool();

        if (m_scopeProxy)
        {
            try
            {
                m_scopeProxy->set_child_scopes(m_child_scopes);
            }
            catch (std::exception const& e)
            {
                qWarning("SettingsModel::settings_timeout: Exception caught from m_scopeProxy->set_child_scopes(): %s", e.what());
                return;
            }
        }
    }
    else if (m_data_by_id.contains(setting_id))
    {
        m_settings->setValue(setting_id, value);
        m_settings->sync(); // make sure the change to setting value is synced to fs
    }
    else
    {
        qWarning() << "No such setting:" << setting_id;
    }

    locker.unlock();
    Q_EMIT settingsChanged();
}
