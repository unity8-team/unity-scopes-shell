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

class Q_DECL_EXPORT GeoIp : public QObject
{
Q_OBJECT

public:
    struct Result
    {
        bool valid = false;

        QString ip;

        QString status;

        QString countryCode;

        QString countryCode3;

        QString countryName;

        QString regionCode;

        QString regionName;

        QString city;

        QString zipPostalCode;

        double latitude = 0.0;

        double longitude = 0.0;

        QString areaCode;

        QString timeZone;
    };

    typedef QSharedPointer<GeoIp> Ptr;

    GeoIp(const QUrl& url = QUrl(QStringLiteral("http://geoip.ubuntu.com/lookup")));

    ~GeoIp() = default;

    void whollyMoveThread(QThread *thread);

public Q_SLOTS:
    void start();

Q_SIGNALS:
    void finished(const Result& result);

protected Q_SLOTS:
    void response(QNetworkReply * const reply);

protected:
    void parseResponse(Result& result, QXmlStreamReader& xml);

    QString readText(QXmlStreamReader& xml);

    QUrl m_url;

    QNetworkAccessManager m_networkAccessManager;

    bool m_running = false;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::GeoIp::Result)

#endif /* GEOIP_H_ */
