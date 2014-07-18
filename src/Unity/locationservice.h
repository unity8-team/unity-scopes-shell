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

#include <com/ubuntu/location/heading.h>
#include <com/ubuntu/location/position.h>
#include <com/ubuntu/location/update.h>
#include <com/ubuntu/location/velocity.h>

#include <unity/scopes/Variant.h>

namespace scopes_ng
{

class Q_DECL_EXPORT LocationService : public QObject
{
    Q_OBJECT

    Q_PROPERTY(unity::scopes::Variant location READ location NOTIFY locationChanged)

    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)

public:
    typedef QSharedPointer<LocationService> Ptr;

    LocationService();

    virtual ~LocationService() = default;

    virtual unity::scopes::Variant location() const = 0;

    virtual bool isActive() const = 0;

public Q_SLOTS:
    /**
     * @brief Initiate a location session. Reference counted.
     */
    virtual void activate() = 0;

    /**
     * @brief End a location session. Reference counted.
     */
    virtual void deactivate() = 0;

Q_SIGNALS:
    void locationChanged();

    void activeChanged();

};

} // namespace scopes_ng

#endif /* LOCATIONSERVICE_H_ */
