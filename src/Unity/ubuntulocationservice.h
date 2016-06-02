/*
 * Copyright (C) 2014-2016 Canonical, Ltd.
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

#ifndef UBUNTULOCATIONSERVICE_H
#define UBUNTULOCATIONSERVICE_H

#include "geoip.h"

#include <QObject>
#include <QSharedPointer>
#include <QTimer>
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <unity/scopes/Location.h>

namespace scopes_ng
{

class Q_DECL_EXPORT UbuntuLocationService: public QObject
{
    Q_OBJECT
    Q_PROPERTY(unity::scopes::Location location READ location NOTIFY locationChanged)
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)

public:
    typedef QSharedPointer<UbuntuLocationService> Ptr;
    class Token : public QObject
    {
    };

    class TokenImpl;

    UbuntuLocationService(const GeoIp::Ptr& geoIp = GeoIp::Ptr(new GeoIp));
    unity::scopes::Location location() const;
    bool hasLocation() const;
    bool isActive() const;
    QSharedPointer<Token> activate();

public Q_SLOTS:
    void requestInitialLocation();

Q_SIGNALS:
    // emited when location changes and only when access has been granted by apparmor
    void locationChanged();

    void locationTimeout();

    // emited when geoip lookup finishes (including initial lookup on startup). regardless of apparmor permissions
    // (receiving it doesn't mean position updates are allowed).
    void geoIpLookupFinished();
    void activeChanged();
    void accessDenied();
    void enqueueActivate();
    void enqueueDeactivate();

protected Q_SLOTS:
    void doActivate();
    void doDeactivate();
    void update();
    void positionChanged(const QGeoPositionInfo& update);
    void onPositionUpdateTimeout();
    void onError(QGeoPositionInfoSource::Error positioningError);
    void requestFinished(const GeoIp::Result& result);

protected:
    bool m_active;
    QGeoPositionInfoSource *m_locationSource;
    QGeoPositionInfo m_lastLocation;
    bool m_locationUpdatedAtLeastOnce = false;
    int m_activationCount = 0;
    QTimer m_geoipTimer;
    QTimer m_deactivateTimer;
    GeoIp::Ptr m_geoIp;
    GeoIp::Result m_result;
};

} // namespace scopes_ng

#endif /* UBUNTULOCATIONSERVICE_H */
