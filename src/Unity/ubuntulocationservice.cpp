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
#include <QMutexLocker>
#include <QRunnable>
#include <QThreadPool>
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

    class DBusThread : public QThread
    {

    public:
        DBusThread(dbus::Bus::Ptr bus) :
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

}

class UbuntuLocationService::Priv : public QObject
{
Q_OBJECT

public:
    Priv() :
            m_lastLocationMutex(QMutex::Recursive), m_resultMutex(
                    QMutex::Recursive)
    {
    }

    void init(GeoIp::Ptr geoIp)
    {
        m_geoIp = geoIp;
        m_geoIp->moveToThread(thread());

        m_deactivateTimer.moveToThread(thread());
        m_deactivateTimer.setInterval(DEACTIVATE_INTERVAL);
        m_deactivateTimer.setSingleShot(true);
        m_deactivateTimer.setTimerType(Qt::VeryCoarseTimer);

        m_geoIp->start();

        try
        {
            m_bus = make_shared<dbus::Bus>(dbus::WellKnownBus::system);
            m_bus->install_executor(dbus::asio::make_executor(m_bus));

            m_dbusThread.reset(new DBusThread(m_bus));
            m_dbusThread->start();

            m_locationService = dbus::resolve_service_on_bus<culs::Interface,
                    culs::Stub>(m_bus);
        }
        catch (exception& e)
        {
            qWarning() << e.what();
        }

        // Wire up the deactivate timer
        connect(&m_deactivateTimer, &QTimer::timeout, this, &Priv::update, Qt::QueuedConnection);

        // Wire up the network request finished timer
        connect(m_geoIp.data(), &GeoIp::finished, this, &Priv::requestFinished, Qt::QueuedConnection);
    }

    ~Priv()
    {
        if (m_bus)
        {
            m_bus->stop();
        }

        if (m_dbusThread && m_dbusThread->isRunning())
        {
            m_dbusThread->wait();
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

        if (m_activationCount > 0)
        {
            // Update the GeoIp data again
            m_geoIp->start();
        }

        try
        {
            if (!m_session)
            {
                m_session = m_locationService->create_session_for_criteria(
                        cul::Criteria());

                m_session->updates().position.changed().connect(
                        bind(&UbuntuLocationService::Priv::positionChanged,
                             this, _1));
            }

            if (m_activationCount > 0
                    && m_session->updates().position_status
                            == culss::Interface::Updates::Status::disabled)
            {
                m_session->updates().position_status =
                        culss::Interface::Updates::Status::enabled;
            }
            else if (m_activationCount == 0
                    && m_session->updates().position_status
                            == culss::Interface::Updates::Status::enabled)
            {
                m_session->updates().position_status =
                        culss::Interface::Updates::Status::disabled;
            }
        }
        catch (exception& e)
        {
            qWarning() << e.what();
        }
    }

    void positionChanged(const cul::Update<cul::Position>& newPosition)
    {
        QMutexLocker lock(&m_lastLocationMutex);

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
        QMutexLocker lock(&m_resultMutex);
        m_result = result;
        Q_EMIT locationChanged();
    }

    void activate()
    {
        ++m_activationCount;
        m_deactivateTimer.stop();
        update();
    }

    void deactivate()
    {
        --m_activationCount;
        if (m_activationCount < 0)
        {
            m_activationCount = 0;
            qWarning() << "Location service refcount error";
        }
        m_deactivateTimer.start();
    }

public:
    dbus::Bus::Ptr m_bus;

    culs::Stub::Ptr m_locationService;

    culss::Interface::Ptr m_session;

    cul::Position m_lastLocation;

    QMutex m_lastLocationMutex;

    bool m_locationUpdatedAtLeastOnce = false;

    int m_activationCount = 0;

    QTimer m_deactivateTimer;

    GeoIp::Ptr m_geoIp;

    QSharedPointer<QThread> m_dbusThread;

    QMutex m_resultMutex;

    GeoIp::Result m_result;
};

UbuntuLocationService::UbuntuLocationService(GeoIp::Ptr geoIp) :
        p(new Priv())
{
    p->moveToThread(&m_thread);

    // If the location service is disabled
    if (qEnvironmentVariableIsSet("UNITY_SCOPES_NO_LOCATION"))
    {
        return;
    }

    p->init(geoIp);

    // Connect to signals (which will be queued)
    connect(p.data(), &Priv::locationChanged, this, &LocationService::locationChanged, Qt::QueuedConnection);
    connect(this, &UbuntuLocationService::enqueueActivate, p.data(), &Priv::activate, Qt::QueuedConnection);
    connect(this, &UbuntuLocationService::enqueueDeactivate, p.data(), &Priv::deactivate, Qt::QueuedConnection);

    m_thread.start();
}

UbuntuLocationService::~UbuntuLocationService()
{
    p.reset();

    m_thread.quit();

    if (m_thread.isRunning())
    {
        m_thread.wait();
    }
}

scopes::Location UbuntuLocationService::location() const
{
    scopes::Location location(0.0, 0.0);

    GeoIp::Result result;
    {
        QMutexLocker lock(&p->m_resultMutex);
        result = p->m_result;
    }

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

    QMutexLocker lock(&p->m_lastLocationMutex);
    // We need to be active, and the location session must have updated at least once
    if (isActive() && p->m_locationUpdatedAtLeastOnce)
    {
        cul::Position pos = p->m_lastLocation;

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
    return p->m_session ? (p->m_session->updates().position_status ==
            culss::Interface::Updates::Status::enabled) : false;
}

bool UbuntuLocationService::hasLocation() const
{
    return p->m_result.valid || p->m_locationUpdatedAtLeastOnce;
}

void UbuntuLocationService::activate()
{
    Q_EMIT enqueueActivate();
}

void UbuntuLocationService::deactivate()
{
    Q_EMIT enqueueDeactivate();
}

#include "ubuntulocationservice.moc"
