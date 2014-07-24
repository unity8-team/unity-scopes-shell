/*
 * Copyright (C) 2011 Canonical, Ltd.
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

// Self
#include "scopes.h"

// Local
#include "scope.h"
#include "ubuntulocationservice.h"

// Qt
#include <QDebug>
#include <QTimer>
#include <QDBusConnection>

#include <unity/scopes/Registry.h>
#include <unity/scopes/Scope.h>
#include <unity/scopes/ScopeProxyFwd.h>
#include <unity/UnityExceptions.h>

namespace scopes_ng
{

using namespace unity;

void ScopeListWorker::run()
{
    try
    {
        // m_runtimeConfig should be null in most cases, and empty string is for system-wide fallback
        if (!m_scopesRuntime) {
            scopes::Runtime::UPtr runtime_uptr = scopes::Runtime::create(m_runtimeConfig.toStdString());
            m_scopesRuntime = std::move(runtime_uptr);
        }
        auto registry = m_scopesRuntime->registry();
        m_metadataMap = registry->list();
    }
    catch (unity::Exception const& err)
    {
        qWarning("ERROR! Caught %s", err.to_string().c_str());
    }
    Q_EMIT discoveryFinished();
}

void ScopeListWorker::setRuntimeConfig(QString const& config)
{
    m_runtimeConfig = config;
}

void ScopeListWorker::setRuntime(scopes::Runtime::SPtr const& runtime)
{
    m_scopesRuntime = runtime;
}

scopes::Runtime::SPtr ScopeListWorker::getRuntime() const
{
    return m_scopesRuntime;
}

scopes::MetadataMap ScopeListWorker::metadataMap() const
{
    return m_metadataMap;
}

int Scopes::LIST_DELAY = -1;

class Scopes::Priv : public QObject {
    Q_OBJECT
Q_SIGNALS:
    void safeInvalidateScopeResults(const QString& scopeName);
public:
    std::unique_ptr<core::ScopedConnection> m_list_update_callback_connection_;
};

Scopes::Scopes(QObject *parent)
    : unity::shell::scopes::ScopesInterface(parent)
    , m_listThread(nullptr)
    , m_loaded(false)
    , m_priv(new Priv())
{
    // delaying spawning the worker thread, causes problems with qmlplugindump
    // without it
    if (LIST_DELAY < 0) {
        QByteArray listDelay = qgetenv("UNITY_SCOPES_LIST_DELAY");
        LIST_DELAY = listDelay.isNull() ? 100 : listDelay.toInt();
    }
    QTimer::singleShot(LIST_DELAY, this, SLOT(populateScopes()));

    connect(m_priv.get(), SIGNAL(safeInvalidateScopeResults(const QString&)), this,
            SLOT(invalidateScopeResults(const QString &)), Qt::QueuedConnection);

    QDBusConnection::sessionBus().connect(QString(), QString("/com/canonical/unity/scopes"), QString("com.canonical.unity.scopes"), QString("InvalidateResults"), this, SLOT(invalidateScopeResults(QString)));

    m_locationService.reset(new UbuntuLocationService());
}

Scopes::~Scopes()
{
    if (m_listThread && !m_listThread->isFinished()) {
        // libunity-scopes supports timeouts, so this shouldn't block forever
        m_listThread->wait();
    }
}

int Scopes::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return m_scopes.count();
}

void Scopes::populateScopes()
{
    auto thread = new ScopeListWorker;
    QByteArray runtimeConfig = qgetenv("UNITY_SCOPES_RUNTIME_PATH");
    thread->setRuntimeConfig(QString::fromLocal8Bit(runtimeConfig));
    QObject::connect(thread, &ScopeListWorker::discoveryFinished, this, &Scopes::discoveryFinished);
    QObject::connect(thread, &ScopeListWorker::finished, thread, &QObject::deleteLater);

    m_listThread = thread;
    m_listThread->start();
}

void Scopes::discoveryFinished()
{
    ScopeListWorker* thread = qobject_cast<ScopeListWorker*>(sender());

    m_scopesRuntime = thread->getRuntime();
    auto scopes = thread->metadataMap();
    m_priv->m_list_update_callback_connection_.reset(
            new core::ScopedConnection(
                    m_scopesRuntime->registry()->set_list_update_callback(
                            std::bind(&Scopes::Priv::safeInvalidateScopeResults,
                                      m_priv.get(), "scopes"))));

    // FIXME: use a dconf setting for this
    QByteArray enabledScopes = qgetenv("UNITY_SCOPES_LIST");

    beginResetModel();

    if (!enabledScopes.isNull()) {
        QList<QByteArray> scopeList = enabledScopes.split(';');
        for (int i = 0; i < scopeList.size(); i++) {
            std::string scope_name(scopeList[i].constData());
            auto it = scopes.find(scope_name);
            if (it != scopes.end()) {
                auto scope = new Scope(this);
                scope->setScopeData(it->second);
                m_scopes.append(scope);
            }
        }
    } else {
        // add all the scopes
        for (auto it = scopes.begin(); it != scopes.end(); ++it) {
            auto scope = new Scope(this);
            scope->setScopeData(it->second);
            m_scopes.append(scope);
        }
    }

    // cache all the metadata
    for (auto it = scopes.begin(); it != scopes.end(); ++it) {
        m_cachedMetadata[QString::fromStdString(it->first)] = std::make_shared<unity::scopes::ScopeMetadata>(it->second);
    }

    endResetModel();

    m_loaded = true;
    Q_EMIT loadedChanged();
    Q_EMIT metadataRefreshed();

    m_listThread = nullptr;
}

void Scopes::refreshFinished()
{
    ScopeListWorker* thread = qobject_cast<ScopeListWorker*>(sender());

    auto scopes = thread->metadataMap();

    // cache all the metadata
    for (auto it = scopes.begin(); it != scopes.end(); ++it) {
        m_cachedMetadata[QString::fromStdString(it->first)] = std::make_shared<unity::scopes::ScopeMetadata>(it->second);
    }

    Q_EMIT metadataRefreshed();

    m_listThread = nullptr;
}

void Scopes::invalidateScopeResults(QString const& scopeName)
{
    // HACK! mediascanner invalidates local media scopes, but those are aggregated, so let's "forward" the call
    if (scopeName == "mediascanner-music") {
        invalidateScopeResults("musicaggregator");
    } else if (scopeName == "mediascanner-video") {
        invalidateScopeResults("videoaggregator");
    } else if (scopeName == "smart-scopes") {
        // emitted when smart scopes proxy discovers new scopes
        Q_FOREACH(Scope* scope, m_scopes) {
            scope->invalidateResults();
        }
    }

    Scope* scope = getScopeById(scopeName);
    if (scope == nullptr) return;

    scope->invalidateResults();
}

QVariant Scopes::data(const QModelIndex& index, int role) const
{
    Scope* scope = m_scopes.at(index.row());

    switch (role) {
        case Scopes::RoleScope:
            return QVariant::fromValue(scope);
        case Scopes::RoleId:
            return QString(scope->id());
        case Scopes::RoleVisible:
            return QVariant::fromValue(scope->visible());
        case Scopes::RoleTitle:
            return QString(scope->name());
        default:
            return QVariant();
    }
}

unity::shell::scopes::ScopeInterface* Scopes::getScope(int row) const
{
    if (row >= m_scopes.size() || row < 0) {
        return nullptr;
    }
    return m_scopes[row];
}

unity::shell::scopes::ScopeInterface* Scopes::getScope(const QString& scopeId) const
{
    return getScopeById(scopeId);
}

Scope* Scopes::getScopeById(QString const& scopeId) const
{
    Q_FOREACH(Scope* scope, m_scopes) {
        if (scope->id() == scopeId) {
            return scope;
        }
    }

    return nullptr;
}

scopes::ScopeMetadata::SPtr Scopes::getCachedMetadata(QString const& scopeId) const
{
    auto it = m_cachedMetadata.constFind(scopeId);
    if (it != m_cachedMetadata.constEnd()) {
        return it.value();
    }

    return scopes::ScopeMetadata::SPtr();
}

void Scopes::refreshScopeMetadata()
{
    // make sure there's just one listing in-progress at any given time
    if (m_listThread == nullptr && m_scopesRuntime) {
        auto thread = new ScopeListWorker;
        thread->setRuntime(m_scopesRuntime);
        QObject::connect(thread, &ScopeListWorker::discoveryFinished, this, &Scopes::refreshFinished);
        QObject::connect(thread, &ScopeListWorker::finished, thread, &QObject::deleteLater);

        m_listThread = thread;
        m_listThread->start();
    }
}

bool Scopes::loaded() const
{
    return m_loaded;
}

LocationService::Ptr Scopes::locationService() const
{
    return m_locationService;
}

} // namespace scopes_ng

#include <scopes.moc>
