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

#include "geoip.h"

#include <QNetworkReply>
#include <QXmlStreamReader>

using namespace scopes_ng;

GeoIp::GeoIp(const QUrl& url)
{
    // Wire up the network request finished signal
    connect(&m_networkAccessManager, &QNetworkAccessManager::finished, this, &GeoIp::response);

    m_networkAccessManager.get(QNetworkRequest(url));
}

void GeoIp::response(QNetworkReply * const reply)
{
    if (reply->error())
    {
        qWarning() << reply->errorString();
        return;
    }

    QXmlStreamReader xml(reply);
    while (!xml.atEnd() && !xml.hasError())
    {
        QXmlStreamReader::TokenType token = xml.readNext();

        /* If token is just StartDocument, we'll go to next.*/
        if (token == QXmlStreamReader::StartDocument)
        {
            continue;
        }

        /* If token is StartElement, we'll see if we can read it.*/
        if (token == QXmlStreamReader::StartElement)
        {
            /* If it's named persons, we'll go to the next.*/
            if (xml.name() == "Response")
            {
                parseResponse(xml);
            }
        }
    }

    if(xml.hasError()) {
        qWarning() << xml.errorString();
        return;
    }

    m_finished = true;
    Q_EMIT finishedChanged();
}

bool GeoIp::finished() const
{
    return m_finished;
}

void GeoIp::parseResponse(QXmlStreamReader& xml)
{
    xml.readNext();

    /*
     * We're going to loop over the things because the order might change.
     * We'll continue the loop until we hit an EndElement named Response.
     */
    while (!(xml.tokenType() == QXmlStreamReader::EndElement
            && xml.name() == "Response"))
    {
        if (xml.tokenType() == QXmlStreamReader::StartElement)
        {
            if (xml.name() == "Ip")
            {
                m_ip = readText(xml);
            }
            else if (xml.name() == "Status")
            {
                m_status = readText(xml);
            }
            else if (xml.name() == "CountryCode")
            {
                m_countryCode = readText(xml);
            }
            else if (xml.name() == "CountryCode3")
            {
                m_countryCode3 = readText(xml);
            }
            else if (xml.name() == "CountryName")
            {
                m_countryName = readText(xml);
            }
            else if (xml.name() == "RegionCode")
            {
                m_regionCode = readText(xml);
            }
            else if (xml.name() == "RegionName")
            {
                m_regionName = readText(xml);
            }
            else if (xml.name() == "City")
            {
                m_city = readText(xml);
            }
            else if (xml.name() == "ZipPostalCode")
            {
                m_zipPostalCode = readText(xml);
            }
            else if (xml.name() == "Latitude")
            {
                m_latitude = readText(xml).toDouble();
            }
            else if (xml.name() == "Longitude")
            {
                m_longitude = readText(xml).toDouble();
            }
            else if (xml.name() == "AreaCode")
            {
                m_areaCode = readText(xml);
            }
            else if (xml.name() == "TimeZone")
            {
                m_timeZone = readText(xml);
            }
        }

        xml.readNext();
    }
}

QString GeoIp::readText(QXmlStreamReader& xml)
{
    xml.readNext();

    if (xml.tokenType() != QXmlStreamReader::Characters)
    {
        return QString();
    }

    return xml.text().toString();
}

const QString & GeoIp::ip() const
{
    return m_ip;
}

const QString & GeoIp::status() const
{
    return m_status;
}

const QString & GeoIp::countryCode() const
{
    return m_countryCode;
}

const QString & GeoIp::countryCode3() const
{
    return m_countryCode3;
}

const QString & GeoIp::countryName() const
{
    return m_countryName;
}

const QString & GeoIp::regionCode() const
{
    return m_regionCode;
}

const QString & GeoIp::regionName() const
{
    return m_regionName;
}

const QString & GeoIp::city() const
{
    return m_city;
}

const QString & GeoIp::zipPostalCode() const
{
    return m_zipPostalCode;
}

double GeoIp::latitude() const
{
    return m_latitude;
}

double GeoIp::longitude() const
{
    return m_longitude;
}

const QString & GeoIp::areaCode() const
{
    return m_areaCode;
}

const QString & GeoIp::timeZone() const
{
    return m_timeZone;
}
