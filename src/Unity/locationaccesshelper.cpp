/*
 * Copyright (C) 2016 Canonical, Ltd.
 *
 * Authors:
 *  Pawel Stolowski <pawel.stolowski@canonical.com>
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

#include "locationaccesshelper.h"
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QDebug>

#define NUM_OF_SEARCHES_BEFORE_LOCATION_PROMPT 6

namespace scopes_ng
{

const QString LocationAccessHelper::scopesLocationDotFile = ".scopesLocationPrompt";

LocationAccessHelper::LocationAccessHelper(QObject *parent) :
    QObject(parent),
    m_numOfSearches(NUM_OF_SEARCHES_BEFORE_LOCATION_PROMPT),
    m_dotFileExists(false),
    m_denied(true)
{
    auto const path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir locationPromptFile(path);
    m_dotFileExists = locationPromptFile.exists(scopesLocationDotFile);
}

bool LocationAccessHelper::shouldRequestLocation() const
{
    return m_dotFileExists || (m_numOfSearches == 0);
}

bool LocationAccessHelper::isLocationAccessDenied() const
{
    return m_denied;
}

void LocationAccessHelper::searchDispatched(QString const& /* scopeId */)
{
    if (m_numOfSearches > 0) {
        --m_numOfSearches;
        qDebug() << "Number of searches before requesting location:" << m_numOfSearches;
    }
}

void LocationAccessHelper::geoIpLookupFinished()
{
    qDebug() << "LocationAccessHelper::geoIpLookupFinished";
    // This signal is not interesting at the moment. If, however we need to refresh scopes on location update,
    // then it should be re-emited (forwarded) here and in positonChanged() below.
}

void LocationAccessHelper::positionChanged()
{
    qDebug() << "LocationAccessHelper::positionChanged";

    if (m_denied) {
        m_denied = false;
        Q_EMIT accessChanged();
    }

    if (!m_dotFileExists) {
        createLocationPromptFile();
    }
}

void LocationAccessHelper::accessDenied()
{
    qDebug() << "LocationAccessHelper::accessDenied";
    if (!m_denied) {
        m_denied = true;
        Q_EMIT accessChanged();
    }

    if (!m_dotFileExists) {
        createLocationPromptFile();
    }
}

void LocationAccessHelper::createLocationPromptFile()
{
    auto const path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/" + scopesLocationDotFile;
    QFile locationPromptFile(path);
    if (locationPromptFile.open(QIODevice::WriteOnly)) {
        qDebug() << "Creating" << locationPromptFile.fileName();
        m_dotFileExists = true;
    } else {
        qWarning() << "Failed to create" << locationPromptFile.fileName();
    }
}

}
