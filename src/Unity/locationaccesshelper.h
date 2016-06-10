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

#pragma once

#include <QObject>

namespace scopes_ng
{

class Q_DECL_EXPORT LocationAccessHelper: public QObject
{
    Q_OBJECT

public:
    LocationAccessHelper(QObject *parent = nullptr);
    void init();

    bool shouldRequestLocation() const;
    bool trustedPromptWasShown() const;
    bool isLocationAccessDenied() const;

public Q_SLOTS:
    void searchDispatched(QString const& scopeId);
    void accessDenied();
    void positionChanged();
    void geoIpLookupFinished();

Q_SIGNALS:
    void accessChanged();
    void requestInitialLocation();

private:
    void createLocationPromptFile();

    int m_numOfSearches;
    bool m_dotFileExists;
    bool m_denied;

    static const QString scopesLocationDotFile;
};

}
