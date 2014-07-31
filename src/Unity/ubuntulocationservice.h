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

#ifndef UBUNTULOCATIONSERVICE_H
#define UBUNTULOCATIONSERVICE_H

#include "geoip.h"
#include "locationservice.h"

#include <QSharedPointer>

namespace scopes_ng
{

class Q_DECL_EXPORT UbuntuLocationService : public LocationService
{
public:
    UbuntuLocationService(GeoIp::Ptr geoIp = GeoIp::Ptr(new GeoIp));

    virtual ~UbuntuLocationService() = default;

    unity::scopes::Location location() const override;

    bool isActive() const override;

    void activate() override;

    void deactivate() override;

protected:
    class Priv;

    QSharedPointer<Priv> p;
};

} // namespace scopes_ng

#endif /* UBUNTULOCATIONSERVICE_H */
