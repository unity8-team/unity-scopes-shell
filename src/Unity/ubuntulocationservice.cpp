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

using namespace std;
using namespace scopes_ng;

namespace scopes = unity::scopes;

namespace
{
    /**
     * The GPS session is terminated this amount of time after
     * moving away from an active scope.
     */
    static const int DEACTIVATE_INTERVAL = 5000;

    /**
     * Re-do the GeoIP call every 60 seconds
     */
    static const int GEOIP_INTERVAL = 60000;
}

class UbuntuLocationService::TokenImpl: public UbuntuLocationService::Token
{
    Q_OBJECT

public:
    TokenImpl(UbuntuLocationService& locationService)
    {
        connect(this, &TokenImpl::destroyed, &locationService, &UbuntuLocationService::enqueueDeactivate);
        Q_EMIT locationService.enqueueActivate();
    }

    ~TokenImpl()
    {
        Q_EMIT destroyed();
    }

Q_SIGNALS:
    void destroyed();
};

UbuntuLocationService::UbuntuLocationService(const GeoIp::Ptr& geoIp)
    : m_geoIp(geoIp)
{
    // If the location service is disabled
    if (qEnvironmentVariableIsSet("UNITY_SCOPES_NO_LOCATION"))
    {
        return;
    }

    m_deactivateTimer.setInterval(DEACTIVATE_INTERVAL);
    m_deactivateTimer.setSingleShot(true);
    m_deactivateTimer.setTimerType(Qt::VeryCoarseTimer);

    m_geoipTimer.setInterval(GEOIP_INTERVAL);
    m_geoipTimer.setTimerType(Qt::CoarseTimer);

    m_locationSource = QGeoPositionInfoSource::createDefaultSource(this);
    connect(m_locationSource, &QGeoPositionInfoSource::positionUpdated, this, &UbuntuLocationService::positionChanged);
    connect(m_locationSource, &QGeoPositionInfoSource::updateTimeout, this, &UbuntuLocationService::onPositionUpdateTimeout);
    connect(m_locationSource, SIGNAL(error(QGeoPositionInfoSource::Error)), this, SLOT(onError(QGeoPositionInfoSource::Error)));

    // Wire up the deactivate timer
    connect(&m_deactivateTimer, &QTimer::timeout, this, &UbuntuLocationService::update);

    // Wire up the network request finished timer
    connect(m_geoIp.data(), &GeoIp::finished, this, &UbuntuLocationService::requestFinished);

    // Wire up the GeoIP repeat timer
    connect(&m_geoipTimer, &QTimer::timeout, m_geoIp.data(), &GeoIp::start);

    // Connect to signals (which will be queued)
    connect(this, &UbuntuLocationService::enqueueActivate, this, &UbuntuLocationService::doActivate, Qt::QueuedConnection);
    connect(this, &UbuntuLocationService::enqueueDeactivate, this, &UbuntuLocationService::doDeactivate, Qt::QueuedConnection);

    m_geoIp->start();
}

void UbuntuLocationService::doActivate()
{
    m_active = true;
    ++m_activationCount;
    m_deactivateTimer.stop();
    update();
}

void UbuntuLocationService::doDeactivate()
{
    --m_activationCount;
    if (m_activationCount < 0)
    {
        m_activationCount = 0;
        qWarning() << "Location service refcount error";
    }
    m_deactivateTimer.start();
}

void UbuntuLocationService::update()
{
    if (m_activationCount > 0)
    {
        // Update the GeoIp data again
        m_geoIp->start();
    }

    try
    {
        if (m_activationCount > 0)
        {
            qDebug() << "Enabling location updates";
            m_locationSource->startUpdates();
            m_geoipTimer.start();
        }
        else
        {
            qDebug() << "Disabling location updates";
            m_active = false;
            m_locationSource->stopUpdates();
            m_geoipTimer.stop();
        }
    }
    catch (exception& e)
    {
        qWarning() << e.what();
    }
}

void UbuntuLocationService::positionChanged(const QGeoPositionInfo& update)
{
    qDebug() << "Position updated:" << update;

    m_locationUpdatedAtLeastOnce = true;
    m_lastLocation = update;
    Q_EMIT locationChanged();
}

void UbuntuLocationService::onPositionUpdateTimeout()
{
    qWarning() << "Position update timeout";
    Q_EMIT locationTimeout();
}

void UbuntuLocationService::onError(QGeoPositionInfoSource::Error positioningError)
{
    qWarning() << "Position update error:" << positioningError;
    if (positioningError == QGeoPositionInfoSource::AccessError) {
        qDebug() << "Postion update denied";
        Q_EMIT accessDenied();
    }
}

void UbuntuLocationService::requestFinished(const GeoIp::Result& result)
{
    qDebug() << "GeoIP request finished";
    {
        m_result = result;
    }
    Q_EMIT geoIpLookupFinished();
}

scopes::Location UbuntuLocationService::location() const
{
    scopes::Location location(0.0, 0.0);

    GeoIp::Result result;
    {
        result = m_result;
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

    // We need to be active, and the location session must have updated at least once
    if (isActive() && m_locationUpdatedAtLeastOnce)
    {
        location.set_latitude(m_lastLocation.coordinate().latitude());
        location.set_longitude(m_lastLocation.coordinate().longitude());
        location.set_altitude(m_lastLocation.coordinate().altitude());

        if (m_lastLocation.hasAttribute(QGeoPositionInfo::HorizontalAccuracy))
        {
            location.set_horizontal_accuracy(m_lastLocation.attribute(QGeoPositionInfo::HorizontalAccuracy));
        }
        if (m_lastLocation.hasAttribute(QGeoPositionInfo::VerticalAccuracy))
        {
            location.set_vertical_accuracy(m_lastLocation.attribute(QGeoPositionInfo::VerticalAccuracy));
        }
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
    return m_active;
}

bool UbuntuLocationService::hasLocation() const
{
    return m_lastLocation.isValid() || m_locationUpdatedAtLeastOnce;
}

QSharedPointer<UbuntuLocationService::Token> UbuntuLocationService::activate()
{
    return QSharedPointer<Token>(new TokenImpl(*this));
}

void UbuntuLocationService::requestInitialLocation()
{
    qDebug() << "Requesting initial location update";
    m_locationSource->requestUpdate();
    m_geoipTimer.start();
}

#include "ubuntulocationservice.moc"
