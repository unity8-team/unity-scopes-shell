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

#include "ubuntulocationservice.h"
#include "geoip.h"

#include <QDebug>
#include <QThread>
#include <QTimer>

#include <com/ubuntu/location/service/stub.h>

#include <core/dbus/resolver.h>
#include <core/dbus/asio/executor.h>

#include <memory>

using namespace std;
using namespace std::placeholders;
using namespace scopes_ng;

namespace cul = com::ubuntu::location;
namespace culs = com::ubuntu::location::service;
namespace culss = com::ubuntu::location::service::session;
namespace culu = com::ubuntu::location::units;
namespace dbus = core::dbus;
namespace scopes = unity::scopes;

namespace
{
    /**
     * The GPS session is terminated this amount of time after
     * moving away from an active scope.
     */
    static const int DEACTIVATE_INTERVAL = 5000;

    /**
     * Minimum time between location updates.
     */
    static const int THROTTLE_INTERVAL = 10000;
}

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
        m_geoIp.start();

        m_bus = make_shared<dbus::Bus>(dbus::WellKnownBus::system);
        m_bus->install_executor(dbus::asio::make_executor(m_bus));

        m_thread.reset(new WorkerThread(m_bus));
        m_thread->start();

        m_locationService = dbus::resolve_service_on_bus<culs::Interface,
                            culs::Stub>(m_bus);

        m_deactivateTimer.setInterval(DEACTIVATE_INTERVAL);
        m_deactivateTimer.setSingleShot(true);
        m_deactivateTimer.setTimerType(Qt::VeryCoarseTimer);

        m_positionUpdateThrottle.setInterval(THROTTLE_INTERVAL);
        m_positionUpdateThrottle.setSingleShot(true);
        m_positionUpdateThrottle.setTimerType(Qt::VeryCoarseTimer);
    }

    ~Priv()
    {
        m_bus->stop();
        m_thread->wait();
        m_thread->exit();
    }

Q_SIGNALS:
    void locationChanged();

public Q_SLOTS:
    void update()
    {
        if (m_refCount > 0 && !m_session)
        {
            // Update the GeoIp data again
            m_geoIp.start();

            // Starting a new location service session
            try
            {
                m_lastLocation.reset();
                m_dirty = false;

                m_session = m_locationService->create_session_for_criteria(
                        cul::Criteria());

                m_session->updates().position.changed().connect(
                        bind(&Priv::positionChanged, this, _1));

                m_session->updates().position_status =
                        culss::Interface::Updates::Status::enabled;
            }
            catch (exception& e)
            {
                qWarning() << e.what();
            }
        }
        else if (m_refCount == 0 && m_session)
        {
            m_session.reset();
        }
    }

    void positionChanged(const cul::Update<cul::Position>& newPosition)
    {
        if (m_lastLocation)
        {
            culu::Quantity<culu::Length> distance = cul::haversine_distance(*m_lastLocation, newPosition.value);
            culu::Quantity<culu::Length> threshold{ 50.0 * culu::Meters };
            if (distance <= threshold)
            {
                return;
            }
        }

        m_lastLocation = make_shared<cul::Position>(newPosition.value);

        // If the timer is already active, we have
        // received an update "soon" after a previous
        // update, so throttle it.
        if (m_positionUpdateThrottle.isActive())
        {
            m_dirty = true;
        }
        // There's no block on the update (it's probably
        // the first) so dispatch right away.
        else
        {
            // Block updates from being dispatched for a
            // fixed time
            m_positionUpdateThrottle.start();
            Q_EMIT locationChanged();
        }
    }

    void throttleTimeout()
    {
        // If we got additional updates while we were
        // throttling, we need to dispatch them now.
        if (m_dirty)
        {
            m_dirty = false;
            Q_EMIT locationChanged();
        }
    }

    void requestFinished(const GeoIp::Result& result)
    {
        m_result = result;
        Q_EMIT locationChanged();
    }

public:
    dbus::Bus::Ptr m_bus;

    culs::Stub::Ptr m_locationService;

    culss::Interface::Ptr m_session;

    shared_ptr<cul::Position> m_lastLocation;

    QScopedPointer<WorkerThread> m_thread;

    int m_refCount = 0;

    QTimer m_deactivateTimer;

    GeoIp m_geoIp;

    GeoIp::Result m_result;

    QTimer m_positionUpdateThrottle;

    bool m_dirty = false;
};

UbuntuLocationService::UbuntuLocationService() :
        p(new Priv(), &QObject::deleteLater)
{
    // Connect to signals (which will be queued)
    connect(p.data(), &Priv::locationChanged, this, &LocationService::locationChanged, Qt::QueuedConnection);

    // Wire up the deactivate timer
    connect(&p->m_deactivateTimer, &QTimer::timeout, p.data(), &Priv::update);

    // Wire up the network request finished timer
    connect(&p->m_geoIp, &GeoIp::finished, p.data(), &Priv::requestFinished);
}

unity::scopes::Variant UbuntuLocationService::location() const
{
    scopes::VariantMap location;

    const GeoIp::Result& result(p->m_result);

    if (result.valid)
    {
        location["countryCode"] = result.countryCode.toStdString();
        location["countryName"] = result.countryName.toStdString();

        location["regionCode"] = result.regionCode.toStdString();
        location["regionName"] = result.regionName.toStdString();

        location["zipPostalCode"] = result.zipPostalCode.toStdString();
        location["areaCode"] = result.areaCode.toStdString();

        location["city"] = result.city.toStdString();
    }

    scopes::VariantMap position;

    // We need to be active, and the location session must have updated at least once
    if (isActive() && p->m_lastLocation)
    {
        const cul::Position& pos = *p->m_lastLocation;

        scopes::VariantMap accuracy;
        if (pos.accuracy.horizontal)
        {
            // location.position.accuracy.horizontal
            accuracy["horizontal"] = pos.accuracy.horizontal.get().value();
        }
        if (pos.accuracy.vertical)
        {
            // location.position.accuracy.vertical
            accuracy["vertical"] = pos.accuracy.vertical.get().value();
        }
        if (pos.accuracy.horizontal || pos.accuracy.horizontal)
        {
            // location.position.accuracy
            position["accuracy"] = accuracy;
        }

        if (pos.altitude)
        {
            // location.position.altitude
            position["altitude"] = pos.altitude.get().value.value();
        }

        // location.position.latitude
        position["latitude"] = pos.latitude.value.value();
        // location.position.longitude
        position["longitude"] = pos.longitude.value.value();
    }
    else if (result.valid)
    {
        scopes::VariantMap accuracy;
        // location.position.accuracy.horizontal
        accuracy["horizontal"] = 100000.0;
        // location.position.accuracy
        position["accuracy"] = accuracy;

        // location.position.latitude
        position["latitude"] = result.latitude;
        // location.position.longitude
        position["longitude"] = result.longitude;
    }
    else
    {
        throw domain_error("Location unavailable");
    }

    // location.position
   location["position"] = position;

    return scopes::Variant(location);
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
