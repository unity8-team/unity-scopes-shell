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
    Priv(GeoIp::Ptr geoIp) :
            m_geoIp(geoIp)
    {
        m_deactivateTimer.setInterval(DEACTIVATE_INTERVAL);
        m_deactivateTimer.setSingleShot(true);
        m_deactivateTimer.setTimerType(Qt::VeryCoarseTimer);

        // If the location service is disabled
        if (qEnvironmentVariableIsSet("UNITY_SCOPES_NO_LOCATION"))
        {
            return;
        }

        m_geoIp->start();

        try
        {
            m_bus = make_shared<dbus::Bus>(dbus::WellKnownBus::system);
            m_bus->install_executor(dbus::asio::make_executor(m_bus));

            m_thread.reset(new WorkerThread(m_bus));
            m_thread->start();

            m_locationService = dbus::resolve_service_on_bus<culs::Interface,
                    culs::Stub>(m_bus);
        }
        catch (exception& e)
        {
            qWarning() << e.what();
        }
    }

    ~Priv()
    {
        if (m_bus)
        {
            m_bus->stop();
        }
        if (m_thread && m_thread->isRunning())
        {
            m_thread->wait();
        }
    }

Q_SIGNALS:
    void locationChanged();

public Q_SLOTS:
    void update()
    {
        if (!m_locationService)
        {
            qWarning() << "Location service not available";
            return;
        }

        if (m_activationCount > 0 && !m_session)
        {
            // Update the GeoIp data again
            m_geoIp->start();

            // Starting a new location service session
            try
            {
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
        else if (m_activationCount == 0 && m_session)
        {
            m_session.reset();
        }
    }

    void positionChanged(const cul::Update<cul::Position>& newPosition)
    {
        if (m_locationUpdatedAtLeastOnce)
        {
            culu::Quantity<culu::Length> distance = cul::haversine_distance(
                    m_lastLocation, newPosition.value);
            culu::Quantity<culu::Length> threshold(50.0 * culu::Meters);

            // Ignore the update if we haven't moved significantly
            if (distance <= threshold)
            {
                return;
            }
        }

        m_locationUpdatedAtLeastOnce = true;
        m_lastLocation = newPosition.value;
        Q_EMIT locationChanged();
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

    cul::Position m_lastLocation;

    bool m_locationUpdatedAtLeastOnce = false;

    QScopedPointer<WorkerThread> m_thread;

    int m_activationCount = 0;

    QTimer m_deactivateTimer;

    GeoIp::Ptr m_geoIp;

    GeoIp::Result m_result;
};

UbuntuLocationService::UbuntuLocationService(GeoIp::Ptr geoIp) :
        p(new Priv(geoIp), &QObject::deleteLater)
{
    // Connect to signals (which will be queued)
    connect(p.data(), &Priv::locationChanged, this, &LocationService::locationChanged, Qt::QueuedConnection);

    // Wire up the deactivate timer
    connect(&p->m_deactivateTimer, &QTimer::timeout, p.data(), &Priv::update);

    // Wire up the network request finished timer
    connect(p->m_geoIp.data(), &GeoIp::finished, p.data(), &Priv::requestFinished);
}

scopes::Location UbuntuLocationService::location() const
{
    scopes::Location location(0.0, 0.0);

    const GeoIp::Result& result(p->m_result);

    if (result.valid)
    {
        location.set_country_code(result.countryCode.toStdString());
        location.set_country_name(result.countryName.toStdString());

        location.set_region_code(result.regionCode.toStdString());
        location.set_region_name(result.regionName.toStdString());

        location.set_zip_postal_code(result.zipPostalCode.toStdString());
        location.set_area_code(result.areaCode.toStdString());

        location.set_city(result.city.toStdString());
    }

    // We need to be active, and the location session must have updated at least once
    if (isActive() && p->m_locationUpdatedAtLeastOnce)
    {
        const cul::Position& pos = p->m_lastLocation;

        if (pos.accuracy.horizontal)
        {
            location.set_horizontal_accuracy(pos.accuracy.horizontal.get().value());
        }
        if (pos.accuracy.vertical)
        {
            location.set_vertical_accuracy(pos.accuracy.vertical.get().value());
        }

        if (pos.altitude)
        {
            location.set_altitude(pos.altitude.get().value.value());
        }

        location.set_latitude(pos.latitude.value.value());
        location.set_longitude(pos.longitude.value.value());
    }
    else if (result.valid)
    {
        location.set_horizontal_accuracy(100000.0);

        location.set_latitude(result.latitude);
        location.set_longitude(result.longitude);
    }
    else
    {
        throw domain_error("Location unavailable");
    }

    return location;
}

bool UbuntuLocationService::isActive() const
{
    return p->m_session ? true : false;
}

void UbuntuLocationService::activate()
{
    ++p->m_activationCount;
    p->m_deactivateTimer.stop();
    p->update();
}

void UbuntuLocationService::deactivate()
{
    --p->m_activationCount;
    if (p->m_activationCount < 0)
    {
        p->m_activationCount = 0;
        qWarning() << "Location service refcount error";
    }
    p->m_deactivateTimer.start();
}

#include "ubuntulocationservice.moc"
