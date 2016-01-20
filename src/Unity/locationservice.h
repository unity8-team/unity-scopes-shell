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

#ifndef LOCATIONSERVICE_H
#define LOCATIONSERVICE_H

#include <QObject>
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>

#include <unity/scopes/Location.h>

namespace scopes_ng
{

class Q_DECL_EXPORT LocationService : public QObject
{
    Q_OBJECT

    Q_PROPERTY(unity::scopes::Location location READ location NOTIFY locationChanged)

    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)

public:
    LocationService(QObject *parent = nullptr);

    virtual ~LocationService() = default;

    virtual unity::scopes::Location location() const;

    virtual bool hasLocation();

    virtual bool isActive();

Q_SIGNALS:
    void locationChanged();

    void activeChanged();

private Q_SLOTS:
    void onPositionUpdated(const QGeoPositionInfo & update);
    void onPositionUpdateTimeout();
    void onError(QGeoPositionInfoSource::Error positioningError);

private:
    QGeoPositionInfoSource *m_locationSource;
    QGeoPositionInfo m_location;
};

} // namespace scopes_ng

#endif /* LOCATIONSERVICE_H_ */
