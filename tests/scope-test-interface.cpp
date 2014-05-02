/*
 * scope-test-interface.cpp
 *
 *  Created on: 30 Apr 2014
 *      Author: pete
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

