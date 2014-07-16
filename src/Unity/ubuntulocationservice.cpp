/*
 * Copyright (C) 2014 Canonical, Ltd.
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
 *
 * Authors:
 *  Pete Woods <pete.woods@canonical.com>
 */

#include <ubuntulocationservice.h>

#include <QDebug>
#include <QThread>
#include <QTimer>

#include <com/ubuntu/location/service/stub.h>

#include <core/dbus/resolver.h>
#include <core/dbus/asio/executor.h>

#include <memory>

using namespace std;
using namespace scopes_ng;

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;
namespace dbus = core::dbus;

class WorkerThread : public QThread
{
Q_OBJECT

public:
    WorkerThread(dbus::Bus::Ptr bus) :
            m_bus(bus)
    {
    }

protected:
    void run() override
    {
        m_bus->run();
    }

    dbus::Bus::Ptr m_bus;
};

class UbuntuLocationService::Priv : public QObject
{
Q_OBJECT

public:
    Priv()
    {
        m_bus = make_shared<dbus::Bus>(dbus::WellKnownBus::system);
        m_bus->install_executor(dbus::asio::make_executor(m_bus));

        m_thread.reset(new WorkerThread(m_bus));
        m_thread->start();

        m_location_service = dbus::resolve_service_on_bus<culs::Interface,
                            culs::Stub>(m_bus);

        m_deactivateTimer.setInterval(5000);
        m_deactivateTimer.setSingleShot(true);
        m_deactivateTimer.setTimerType(Qt::VeryCoarseTimer);
    }

    ~Priv()
    {
        m_bus->stop();
        m_thread->wait();
        m_thread->exit();
    }

Q_SIGNALS:
    void positionChanged();

    void velocityChanged();

    void headingChanged();

public Q_SLOTS:
    void update()
    {
        if (m_refCount > 0 && !m_session)
        {
            qDebug() << "starting location session";
            try
            {
                m_session = m_location_service->create_session_for_criteria(
                        cul::Criteria());

                m_session->updates().position.changed().connect(
                        bind(&Priv::positionChanged, this));
                m_session->updates().velocity.changed().connect(
                        bind(&Priv::velocityChanged, this));
                m_session->updates().heading.changed().connect(
                        bind(&Priv::headingChanged, this));

                m_session->updates().position_status =
                        culss::Interface::Updates::Status::enabled;
                m_session->updates().heading_status =
                        culss::Interface::Updates::Status::enabled;
                m_session->updates().velocity_status =
                        culss::Interface::Updates::Status::enabled;

            }
            catch (exception& e)
            {
                qWarning() << e.what();
            }
        }
        else if (m_refCount == 0 && m_session)
        {
            qDebug() << "ending location session";
            m_session.reset();
        }
    }

public:
    dbus::Bus::Ptr m_bus;

    culs::Stub::Ptr m_location_service;

    culss::Interface::Ptr m_session;

    QScopedPointer<WorkerThread> m_thread;

    int m_refCount = 0;

    QTimer m_deactivateTimer;
};

UbuntuLocationService::UbuntuLocationService() :
        p(new Priv(), &QObject::deleteLater)
{
    // Connect to signals (which will be queued)
    connect(p.data(), &Priv::positionChanged, this, &LocationService::positionChanged, Qt::QueuedConnection);
    connect(p.data(), &Priv::velocityChanged, this, &LocationService::velocityChanged, Qt::QueuedConnection);
    connect(p.data(), &Priv::headingChanged, this, &LocationService::headingChanged, Qt::QueuedConnection);

    // Wire up the deactivate timer
    connect(&p->m_deactivateTimer, &QTimer::timeout, p.data(), &Priv::update);
}

cul::Position UbuntuLocationService::position() const
{
    if (!isActive())
    {
        throw domain_error("No active session");
    }
    return p->m_session->updates().position.get().value;
}

cul::Velocity UbuntuLocationService::velocity() const
{
    if (!isActive())
    {
        throw domain_error("No active session");
    }
    return p->m_session->updates().velocity.get().value;
}

cul::Heading UbuntuLocationService::heading() const
{
    if (!isActive())
    {
        throw domain_error("No active session");
    }
    return p->m_session->updates().heading.get().value;
}

bool UbuntuLocationService::isActive() const
{
    return p->m_session ? true : false;
}

void UbuntuLocationService::activate()
{
    ++p->m_refCount;
    p->m_deactivateTimer.stop();
    p->update();
}

void UbuntuLocationService::deactivate()
{
    --p->m_refCount;
    if (p->m_refCount < 0)
    {
        p->m_refCount = 0;
        qWarning() << "Location service refcount error";
    }
    p->m_deactivateTimer.start();
}

#include "ubuntulocationservice.moc"
