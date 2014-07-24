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

GeoIp::GeoIp(const QUrl& url) :
        m_url(url)
{
    // Wire up the network request finished signal
    connect(&m_networkAccessManager, &QNetworkAccessManager::finished, this, &GeoIp::response);
}

void GeoIp::start()
{
    if (!m_running)
    {
        m_running = true;
        m_networkAccessManager.get(QNetworkRequest(m_url));
    }
}

void GeoIp::response(QNetworkReply * const reply)
{
    m_running = false;

    if (reply->error())
    {
        qWarning() << reply->errorString();
        return;
    }

    Result result;

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
                parseResponse(result, xml);
            }
        }
    }

    if (xml.hasError())
    {
        qWarning() << xml.errorString();
    }
    else
    {
        result.valid = true;
    }

    Q_EMIT finished(result);
}

void GeoIp::parseResponse(Result& result, QXmlStreamReader& xml)
{
    xml.readNext();

    /*
     * We're going to loop over the things because the order might change.
     * We'll continue the loop until we hit an EndElement named Response.
     */
    while (!xml.atEnd() && !(xml.isEndElement() && xml.name() == "Response"))
    {
        if (xml.isStartElement())
        {
            if (xml.name() == "Ip")
            {
                result.ip = readText(xml);
            }
            else if (xml.name() == "Status")
            {
                result.status = readText(xml);
            }
            else if (xml.name() == "CountryCode")
            {
                result.countryCode = readText(xml);
            }
            else if (xml.name() == "CountryCode3")
            {
                result.countryCode3 = readText(xml);
            }
            else if (xml.name() == "CountryName")
            {
                result.countryName = readText(xml);
            }
            else if (xml.name() == "RegionCode")
            {
                result.regionCode = readText(xml);
            }
            else if (xml.name() == "RegionName")
            {
                result.regionName = readText(xml);
            }
            else if (xml.name() == "City")
            {
                result.city = readText(xml);
            }
            else if (xml.name() == "ZipPostalCode")
            {
                result.zipPostalCode = readText(xml);
            }
            else if (xml.name() == "Latitude")
            {
                result.latitude = readText(xml).toDouble();
            }
            else if (xml.name() == "Longitude")
            {
                result.longitude = readText(xml).toDouble();
            }
            else if (xml.name() == "AreaCode")
            {
                result.areaCode = readText(xml);
            }
            else if (xml.name() == "TimeZone")
            {
                result.timeZone = readText(xml);
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
