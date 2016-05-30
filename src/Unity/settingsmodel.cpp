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

#include <unity/util/ResourcePtr.h>
#include <unity/UnityExceptions.h>

#include <QDebug>
#include <QDir>
#include <QTextCodec>
#include <QTimer>

#include <fcntl.h>
#include <unistd.h>

using namespace scopes_ng;
namespace sc = unity::scopes;

namespace
{

typedef unity::util::ResourcePtr<int, std::function<void(int)>> FileLock;

static FileLock unixLock(const QString& path, bool writeLock)
{
    FileLock fileLock(::open(path.toUtf8(), writeLock ? O_WRONLY : O_RDONLY), [](int fd)
    {
        if (fd != -1)
        {
            close(fd);
        }
    });

    if (fileLock.get() == -1)
    {
        throw unity::FileException("Couldn't open file " + path.toStdString(), errno);
    }

    struct flock fl;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_type = writeLock ? F_WRLCK : F_RDLCK;

    if (::fcntl(fileLock.get(), F_SETLKW, &fl) != 0)
    {
        throw unity::FileException("Couldn't get file lock for " + path.toStdString(), errno);
    }

    return fileLock;
}

static const char* GROUP_NAME = "General";

}  // namespace

SettingsModel::SettingsModel(const QDir& configDir, const QString& scopeId,
        const QVariant& settingsDefinitions, bool isLocationGloballyEnabled,
        QObject* parent,
        int settingsTimeout)
        : SettingsModelInterface(parent), m_scopeId(scopeId), m_settingsTimeout(settingsTimeout),
          m_requireChildScopesRefresh(false)
{
    configDir.mkpath(scopeId);
    QDir databaseDir = configDir.filePath(scopeId);

    m_settings_path = databaseDir.filePath(QStringLiteral("settings.ini"));

    try
    {
        tryLoadSettings(true);
    }
    catch(const unity::FileException&)
    {
        // No settings file found, at this point we'll just have to continue with a null m_settings.
    }

    for (const auto &it : settingsDefinitions.toList())
    {
        QVariantMap data = it.toMap();
        QString id = data[QStringLiteral("id")].toString();
        QString displayName = data[QStringLiteral("displayName")].toString();

        if (id == "internal.location" && !isLocationGloballyEnabled) {
            qDebug() << "Location setting ignored, waiting for global location access to be enabled first";
            continue;
        }

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
                try
                {
                    tryLoadSettings(true);
                    switch (data->variantType)
                    {
                        case QVariant::Bool:
                            result = m_settings->get_boolean(GROUP_NAME, data->id.toStdString());
                            break;
                        case QVariant::UInt:
                            result = m_settings->get_int(GROUP_NAME, data->id.toStdString());
                            break;
                        case QVariant::Double:
                            result = m_settings->get_double(GROUP_NAME, data->id.toStdString());
                            break;
                        case QVariant::String:
                            result = m_settings->get_string(GROUP_NAME, data->id.toStdString()).c_str();
                            break;
                        default:
                            result = data->defaultValue;
                    }
                }
                catch(const unity::FileException& e)
                {
                    qWarning() << "SettingsModel::data: Failed to read settings file:" << e.what();
                    result = data->defaultValue;
                }
                catch(const unity::LogicException&)
                {
                    qWarning() << "SettingsModel::data: Failed to get a value for setting:" << data->id;
                    result = data->defaultValue;
                }
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
        QVariant result;
        try
        {
            tryLoadSettings(true);
            switch (data->variantType)
            {
                case QVariant::Bool:
                    result = m_settings->get_boolean(GROUP_NAME, data->id.toStdString());
                    break;
                case QVariant::UInt:
                    result = m_settings->get_int(GROUP_NAME, data->id.toStdString());
                    break;
                case QVariant::Double:
                    result = m_settings->get_double(GROUP_NAME, data->id.toStdString());
                    break;
                case QVariant::String:
                    result = m_settings->get_string(GROUP_NAME, data->id.toStdString()).c_str();
                    break;
                default:
                    result = data->defaultValue;
            }
        }
        catch(const unity::FileException& e)
        {
            qWarning() << "SettingsModel::value: Failed to read settings file:" << e.what();
            result = data->defaultValue;
        }
        catch(const unity::LogicException&)
        {
            qWarning() << "SettingsModel::value: Failed to get a value for setting:" << data->id;
            result = data->defaultValue;
        }
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

        QSharedPointer<Data> setting(
                new Data(id, displayName, QStringLiteral("boolean"), QVariantMap(), QVariant(), QVariant::Bool));

        m_child_scopes_data << setting;
        m_child_scopes_data_by_id[id] = setting;
    }

    if (reset) {
        endResetModel();
    }

    locker.unlock();
    Q_EMIT countChanged();
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
                if (!m_child_scopes_timers.contains(data->id))
                {
                    QSharedPointer<QTimer> timer(new QTimer());
                    timer->setProperty("setting_id", data->id);
                    timer->setSingleShot(true);
                    timer->setInterval(m_settingsTimeout);
                    timer->setTimerType(Qt::VeryCoarseTimer);
                    connect(timer.data(), SIGNAL(timeout()), this,
                            SLOT(settings_timeout()));
                    m_child_scopes_timers[data->id] = timer;
                }

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

                locker.unlock();
                Q_EMIT settingsChanged();
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
        try
        {
            tryLoadSettings(false);
            switch (value.type())
            {
                case QVariant::Bool:
                    m_settings->set_boolean(GROUP_NAME, setting_id.toStdString(), value.toBool());
                    break;
                case QVariant::Int:
                case QVariant::UInt:
                    m_settings->set_int(GROUP_NAME, setting_id.toStdString(), value.toUInt());
                    break;
                case QVariant::Double:
                    m_settings->set_double(GROUP_NAME, setting_id.toStdString(), value.toDouble());
                    break;
                case QVariant::String:
                    m_settings->set_string(GROUP_NAME, setting_id.toStdString(), value.toString().toStdString());
                    break;
                default:
                    qWarning() << "SettingsModel::settings_timeout: Invalid value type for setting:" << setting_id;
            }
            FileLock lock = unixLock(m_settings_path, true);
            m_settings->sync(); // make sure the change to setting value is synced to fs

            locker.unlock();
            Q_EMIT settingsChanged();
        }
        catch(const unity::FileException& e)
        {
            qWarning() << "SettingsModel::settings_timeout: Failed to write settings file:" << e.what();
        }
        catch(const unity::LogicException&)
        {
            qWarning() << "SettingsModel::settings_timeout: Failed to set a value for setting:" << setting_id;
        }
    }
    else
    {
        qWarning() << "No such setting:" << setting_id;
    }
}

void SettingsModel::tryLoadSettings(bool read_only) const
{
    if (!m_settings)
    {
        QFileInfo checkFile(m_settings_path);
        if (!checkFile.exists() || !checkFile.isFile())
        {
            if (read_only)
            {
                throw unity::FileException("Could not locate a settings file at: " + m_settings_path.toStdString(), -1);
            }
            // Config file does not exist, so we create an empty one.
            else if (!QFile(m_settings_path).open(QFile::WriteOnly))
            {
                throw unity::FileException("Could not create an empty settings file at: " + m_settings_path.toStdString(), -1);
            }
        }

        FileLock lock = unixLock(m_settings_path, false);
        m_settings.reset(new unity::util::IniParser(m_settings_path.toUtf8()));
    }
}
