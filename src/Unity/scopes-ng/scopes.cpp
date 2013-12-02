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

// Qt
#include <QDebug>
#include <QTimer>

#include <scopes/Registry.h>
#include <scopes/Scope.h>
#include <scopes/ScopeProxyFwd.h>
#include <unity/UnityExceptions.h>

namespace scopes_ng
{

using namespace unity::api;

void ScopeListWorker::run()
{
    try
    {
        // FIXME: use proper path for the runtime config
        //   but have libunity-scopes export it first?!
        m_scopesRuntime = scopes::Runtime::create("dash");
        auto registry = m_scopesRuntime->registry();
        m_scopeMap = registry->list();
    }
    catch (unity::Exception const& err)
    {
        qWarning("ERROR! Caught %s", err.to_string().c_str());
    }
    Q_EMIT discoveryFinished();
}

scopes::Runtime::UPtr ScopeListWorker::takeRuntime()
{
    return std::move(m_scopesRuntime);
}

scopes::ScopeMap ScopeListWorker::scopeMap() const
{
    return m_scopeMap;
}
        
Scopes::Scopes(QObject *parent)
    : QAbstractListModel(parent)
    , m_listThread(nullptr)
    , m_loaded(false)
{
    m_roles[Scopes::RoleScope] = "scope";
    m_roles[Scopes::RoleId] = "id";
    m_roles[Scopes::RoleVisible] = "visible";
    m_roles[Scopes::RoleTitle] = "title";

    // delaying spawning the worker thread, causes problems with qmlplugindump
    // without it
    QTimer::singleShot(100, this, SLOT(populateScopes()));
}

Scopes::~Scopes()
{
    if (m_listThread && !m_listThread->isFinished()) {
        // FIXME: wait indefinitely once libunity-scopes supports timeouts
        m_listThread->wait(5000);
    }
}

QHash<int, QByteArray> Scopes::roleNames() const
{
    return m_roles;
}

int Scopes::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    return m_scopes.count();
}

void Scopes::populateScopes()
{
    auto thread = new ScopeListWorker;
    QObject::connect(thread, &ScopeListWorker::discoveryFinished, this, &Scopes::discoveryFinished);
    QObject::connect(thread, &ScopeListWorker::finished, thread, &QObject::deleteLater);

    m_listThread = thread;
    m_listThread->start();
}

void Scopes::discoveryFinished()
{
    ScopeListWorker* thread = qobject_cast<ScopeListWorker*>(sender());

    m_scopesRuntime = thread->takeRuntime();
    auto scopes = thread->scopeMap();

    beginResetModel();
    for (auto it = scopes.begin(); it != scopes.end(); ++it) {
        auto scope = new Scope(this);
        scope->setScopeData(it->second);
        m_scopes.append(scope);
    }
    endResetModel();

    m_loaded = true;
    Q_EMIT loadedChanged(m_loaded);

    m_listThread = nullptr;
}

QVariant Scopes::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() >= m_scopes.count()) {
        return QVariant();
    }

    Scope* scope = m_scopes.at(index.row());

    switch (role) {
        case Scopes::RoleScope:
            return QVariant::fromValue(scope);
        case Scopes::RoleId:
            return QVariant::fromValue(scope->id());
        case Scopes::RoleVisible:
            return QVariant::fromValue(scope->visible());
        case Scopes::RoleTitle:
            return QVariant::fromValue(scope->name());
        default:
            return QVariant();
    }
}

QVariant Scopes::get(int row) const
{
    return data(QAbstractListModel::index(row), 0);
}

QVariant Scopes::get(const QString& scope_id) const
{
    Q_FOREACH(Scope* scope, m_scopes) {
        if (scope->id() == scope_id) {
            return QVariant::fromValue(scope);
        }
    }

    return QVariant();
}

bool Scopes::loaded() const
{
    return m_loaded;
}

} // namespace scopes_ng
