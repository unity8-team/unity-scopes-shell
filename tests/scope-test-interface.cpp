/*
 * Copyright (C) 2014 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Pete Woods <pete.woods@canonical.com>
 */

#include <tests/scope-test-interface.h>
#include <TestInterfaceAdaptor.h>
#include <QDBusConnection>

ScopeTestInterface::ScopeTestInterface() :
        m_adaptor(new TestAdaptor(this))
{
    qDebug()
            << QDBusConnection::sessionBus().registerObject(
                    "/com/canonical/scopes/test", this);
    qDebug()
            << QDBusConnection::sessionBus().registerService(
                    "com.canonical.scopes.test");
}

ScopeTestInterface::~ScopeTestInterface()
{
    QDBusConnection::sessionBus().unregisterObject(
            "/com/canonical/scopes/test");
    qDebug()
            << QDBusConnection::sessionBus().unregisterService(
                    "com.canonical.scopes.test");
}

