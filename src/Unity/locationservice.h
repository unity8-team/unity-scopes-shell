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

#ifndef LOCATIONSERVICE_H
#define LOCATIONSERVICE_H

#include <QObject>

#include <unity/scopes/Location.h>

namespace scopes_ng
{

class Q_DECL_EXPORT LocationService : public QObject
{
    Q_OBJECT

    Q_PROPERTY(unity::scopes::Location location READ location NOTIFY locationChanged)

    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)

public:
    typedef QSharedPointer<LocationService> Ptr;

    class Token : public QObject
    {
    };

    LocationService();

    virtual ~LocationService() = default;

    virtual unity::scopes::Location location() const = 0;

    virtual bool hasLocation() const = 0;

    virtual bool isActive() const = 0;

    virtual QSharedPointer<Token> activate() = 0;

public Q_SLOTS:
    void requestInitialLocation() {}

Q_SIGNALS:
    // emited when location changes and only when access has been granted by apparmor
    void locationChanged();

    void locationTimeout();

    // emited when geoip lookup finishes (including initial lookup on startup). regardless of apparmor permissions
    // (receiving it doesn't mean position updates are allowed).
    void geoIpLookupFinished();

    void activeChanged();

    void accessDenied();
};

} // namespace scopes_ng

#endif /* LOCATIONSERVICE_H_ */
