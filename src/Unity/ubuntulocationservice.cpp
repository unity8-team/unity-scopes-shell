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
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>

#include <memory>

using namespace std;
using namespace std::placeholders;
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

class UbuntuLocationService::TokenImpl: public LocationService::Token
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

class UbuntuLocationService::Priv : public QObject
{
Q_OBJECT

public:
    Priv() :
        m_resultMutex(QMutex::Recursive)
    {
    }

    void init(const GeoIp::Ptr& geoIp)
    {
        m_geoIp = geoIp;
        m_geoIp->whollyMoveThread(thread());

        m_deactivateTimer.moveToThread(thread());
        m_deactivateTimer.setInterval(DEACTIVATE_INTERVAL);
        m_deactivateTimer.setSingleShot(true);
        m_deactivateTimer.setTimerType(Qt::VeryCoarseTimer);

        m_geoipTimer.moveToThread(thread());
        m_geoipTimer.setInterval(GEOIP_INTERVAL);
        m_geoipTimer.setTimerType(Qt::CoarseTimer);

        QMetaObject::invokeMethod(m_geoIp.data(), "start", Qt::QueuedConnection);

        m_locationSource = QGeoPositionInfoSource::createDefaultSource(this);
        connect(m_locationSource, &QGeoPositionInfoSource::positionUpdated, this, &Priv::positionChanged);
        connect(m_locationSource, &QGeoPositionInfoSource::positionUpdated, this, &Priv::locationChanged);
        connect(m_locationSource, &QGeoPositionInfoSource::updateTimeout, this, &Priv::onPositionUpdateTimeout);
        connect(m_locationSource, SIGNAL(error(QGeoPositionInfoSource::Error)), this, SLOT(onError));

        // Wire up the deactivate timer
        connect(&m_deactivateTimer, &QTimer::timeout, this, &Priv::update, Qt::QueuedConnection);

        // Wire up the network request finished timer
        connect(m_geoIp.data(), &GeoIp::finished, this, &Priv::requestFinished, Qt::QueuedConnection);

        // Wire up the GeoIP repeat timer
        connect(&m_geoipTimer, &QTimer::timeout, m_geoIp.data(), &GeoIp::start, Qt::QueuedConnection);
    }

    ~Priv()
    {
    }

Q_SIGNALS:
    void locationChanged();

public Q_SLOTS:
    void update()
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

    void positionChanged(const QGeoPositionInfo& update)
    {
        qDebug() << "Position updated:" << update;

        m_locationUpdatedAtLeastOnce = true;
        m_lastLocation = update;
        Q_EMIT locationChanged();
    }

    void onPositionUpdateTimeout()
    {
        qWarning() << "Position update timeout";
    }

    void onError(QGeoPositionInfoSource::Error positioningError)
    {
        qWarning() << "Position update error:" << positioningError;
        if (positioningError == QGeoPositionInfoSource::AccessError) {
            qDebug() << "Postion update denied";
        }
    }

    void requestFinished(const GeoIp::Result& result)
    {
        QMutexLocker lock(&m_resultMutex);
        m_result = result;
        Q_EMIT locationChanged();
    }

    void activate()
    {
        m_active = true;
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
    bool m_active;

    QGeoPositionInfoSource *m_locationSource;
    QGeoPositionInfo m_lastLocation;

    bool m_locationUpdatedAtLeastOnce = false;

    int m_activationCount = 0;

    QTimer m_geoipTimer;

    QTimer m_deactivateTimer;

    GeoIp::Ptr m_geoIp;

    QMutex m_resultMutex;

    GeoIp::Result m_result;
};

UbuntuLocationService::UbuntuLocationService(const GeoIp::Ptr& geoIp) :
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

    // We need to be active, and the location session must have updated at least once
    if (isActive() && p->m_locationUpdatedAtLeastOnce)
    {
        location.set_latitude(p->m_lastLocation.coordinate().latitude());
        location.set_longitude(p->m_lastLocation.coordinate().longitude());
        location.set_altitude(p->m_lastLocation.coordinate().altitude());

        /*TODO if (pos.accuracy.horizontal)
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
        }*/
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
    return p->m_active;
}

bool UbuntuLocationService::hasLocation() const
{
    return p->m_lastLocation.isValid() || p->m_locationUpdatedAtLeastOnce;
}

QSharedPointer<LocationService::Token> UbuntuLocationService::activate()
{
    return QSharedPointer<Token>(new TokenImpl(*this));
}

#include "ubuntulocationservice.moc"
