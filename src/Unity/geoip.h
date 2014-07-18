/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *  Pete Woods <pete.woods@canonical.com>
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
 */

#ifndef GEOIP_H
#define GEOIP_H

#include <QNetworkAccessManager>
#include <QUrl>

QT_BEGIN_NAMESPACE
class QNetworkReply;
class QXmlStreamReader;
QT_END_NAMESPACE

namespace scopes_ng
{

class GeoIp : public QObject
{
Q_OBJECT

public:
    typedef QSharedPointer<GeoIp> Ptr;

    GeoIp(const QUrl& url = QUrl("http://geoip.ubuntu.com/lookup"));

    const QString & ip() const;

    const QString & status() const;

    const QString & countryCode() const;

    const QString & countryCode3() const;

    const QString & countryName() const;

    const QString & regionCode() const;

    const QString & regionName() const;

    const QString & city() const;

    const QString & zipPostalCode() const;

    double latitude() const;

    double longitude() const;

    const QString & areaCode() const;

    const QString & timeZone() const;

    bool finished() const;

Q_SIGNALS:
    void finishedChanged();

protected Q_SLOTS:
    void response(QNetworkReply * const reply);

protected:
    void parseResponse(QXmlStreamReader& xml);

    QString readText(QXmlStreamReader& xml);

    QNetworkAccessManager m_networkAccessManager;

    bool m_finished = false;

    QString m_ip;

    QString m_status;

    QString m_countryCode;

    QString m_countryCode3;

    QString m_countryName;

    QString m_regionCode;

    QString m_regionName;

    QString m_city;

    QString m_zipPostalCode;

    double m_latitude = 0.0;

    double m_longitude = 0.0;

    QString m_areaCode;

    QString m_timeZone;
};

} // namespace scopes_ng

#endif /* GEOIP_H_ */
