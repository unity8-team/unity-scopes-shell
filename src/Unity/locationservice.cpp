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
 *  Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <locationservice.h>
#include <unity/scopes/Location.h>

#include <QDebug>

using namespace scopes_ng;
namespace scopes = unity::scopes;

LocationService::LocationService(QObject *parent)
    : QObject(parent)
{
    m_locationSource = QGeoPositionInfoSource::createDefaultSource(this);
    connect(m_locationSource, &QGeoPositionInfoSource::positionUpdated, this, &LocationService::onPositionUpdated);
    connect(m_locationSource, &QGeoPositionInfoSource::positionUpdated, this, &LocationService::locationChanged);
    connect(m_locationSource, &QGeoPositionInfoSource::updateTimeout, this, &LocationService::onPositionUpdateTimeout);
    connect(m_locationSource, SIGNAL(error(QGeoPositionInfoSource::Error)), this, SLOT(onError));
    m_locationSource->startUpdates();
}

void LocationService::onPositionUpdated(const QGeoPositionInfo& update)
{
    qDebug() << "Position updated:" << update;
    m_location = update;
}

void LocationService::onPositionUpdateTimeout()
{
    qDebug() << "Position update timeout";
}

void LocationService::onError(QGeoPositionInfoSource::Error positioningError)
{
    qDebug() << "Error getting position:" << positioningError;
}

unity::scopes::Location LocationService::location() const
{
    scopes::Location location(0.0, 0.0);
    location.set_latitude(m_location.coordinate().latitude());
    location.set_longitude(m_location.coordinate().longitude());
    location.set_altitude(m_location.coordinate().altitude());
    return location;
}

bool LocationService::hasLocation()
{
    return m_location.isValid();
}

bool LocationService::isActive()
{
    return false;
}
