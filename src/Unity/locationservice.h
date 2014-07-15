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

namespace scopes_ng
{

class Q_DECL_EXPORT LocationService : public QObject
{
    Q_OBJECT

    Q_PROPERTY(com::ubuntu::location::Position position READ position NOTIFY positionChanged)

    Q_PROPERTY(com::ubuntu::location::Velocity velocity READ velocity NOTIFY velocityChanged)

    Q_PROPERTY(com::ubuntu::location::Heading heading READ heading NOTIFY headingChanged)

public:
    LocationService();

    virtual ~LocationService() = default;

    virtual com::ubuntu::location::Position position() const = 0;

    virtual com::ubuntu::location::Velocity velocity() const = 0;

    virtual com::ubuntu::location::Heading heading() const = 0;

Q_SIGNALS:
    void positionChanged();

    void velocityChanged();

    void headingChanged();

};

} // namespace scopes_ng

#endif /* LOCATIONSERVICE_H_ */
