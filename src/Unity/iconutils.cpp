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

QString uriToThumbnailerProviderString(QString const &uri, QString const &mimetype, QVariantHash const &metadata)
{
    if (uri.startsWith(QLatin1String("file:///")) || uri.startsWith(QLatin1String("album://"))) {
        bool isAudio = mimetype.startsWith(QLatin1String("audio/"));
        QString thumbnailerUri;
        if (isAudio) {
            thumbnailerUri = BASE_ALBUMART_URI;
            if (metadata.contains("content")) {
                QVariantHash contentHash = metadata["content"].toHash();
                if (contentHash.contains("content")) { // nested content in Home?
                    contentHash = contentHash["content"].toHash();
                }
                if (contentHash.contains("album") && contentHash.contains("artist")) {
                    QUrlQuery query;
                    query.addQueryItem(QStringLiteral("artist"), contentHash["artist"].toString());
                    query.addQueryItem(QStringLiteral("album"), contentHash["album"].toString());
                    thumbnailerUri.append(query.toString());
                }
            }
        } else {
            thumbnailerUri = BASE_THUMBNAILER_URI;
            thumbnailerUri.append(uri.midRef(7));
        }
        return thumbnailerUri;
    }

    return QString::null;
}
