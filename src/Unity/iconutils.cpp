/*
 * Copyright (C) 2013 Canonical, Ltd.
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
 *  Michal Hruby <michal.hruby@canonical.com>
 */

#include "iconutils.h"

#include <QStringList>
#include <QUrl>
#include <QUrlQuery>

#define BASE_THEME_ICON_URI "image://theme/"
#define BASE_THUMBNAILER_URI "image://thumbnailer/"
#define BASE_ALBUMART_URI "image://albumart/"

QString uriToThumbnailerProviderString(QString const &uri, QVariantHash const &metadata)
{
    if (uri.startsWith(QLatin1String("file:///")) || uri.startsWith(QLatin1String("album://"))) {
        bool isAlbum = metadata.contains(QStringLiteral("album")) && metadata.contains(QStringLiteral("artist"));
        QString thumbnailerUri;
        if (isAlbum) {
            thumbnailerUri = QStringLiteral(BASE_ALBUMART_URI);
            QUrlQuery query;
            query.addQueryItem(QStringLiteral("artist"), metadata[QStringLiteral("artist")].toString());
            query.addQueryItem(QStringLiteral("album"), metadata[QStringLiteral("album")].toString());
            thumbnailerUri.append(query.toString());
        } else {
            thumbnailerUri = QStringLiteral(BASE_THUMBNAILER_URI);
            thumbnailerUri.append(uri.midRef(7));
        }
        return thumbnailerUri;
    }

    return QString::null;
}
