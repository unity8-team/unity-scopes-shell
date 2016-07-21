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

#include <QGSettings>
#include <QVariant>
#include <QDebug>
#include <unity/scopes/CannedQuery.h>
#include <unity/UnityExceptions.h>
#include <algorithm>
#include "favorites.h"

namespace scopes_ng
{

Favorites::Favorites(QObject *parent, QGSettings *dashSettings)
    : QObject(parent),
      m_dashSettings(dashSettings)
{
    if (m_dashSettings) {
        readFavoritesFromGSettings();
        QObject::connect(m_dashSettings, &QGSettings::changed, this, &Favorites::dashSettingsChanged);
    }
}

void Favorites::readFavoritesFromGSettings()
{
    m_favoriteScopes.clear();
    m_positionLookup.clear();

    int pos = 0 ;
    auto const favs = m_dashSettings->get(QStringLiteral("favoriteScopes")).toList();
    for (auto const fv: favs) {
        try
        {
            auto const query = unity::scopes::CannedQuery::from_uri(fv.toString().toStdString());
            auto scopeId = QString::fromStdString(query.scope_id());
            m_favoriteScopes.append(scopeId);
            m_positionLookup[scopeId] = pos++;
        }
        catch (const unity::InvalidArgumentException &e)
        {
            qWarning() << "Invalid canned query '" << fv.toString() << "'" << QString::fromStdString(e.what());
        }
    }
}

Favorites::~Favorites()
{
}

int Favorites::setFavorite(QString const& scopeId, bool value)
{
    if (!value) {
        int pos = position(scopeId);
        if (pos >= 0) {
            m_favoriteScopes.removeAt(pos);
            for (int i = pos; i<m_favoriteScopes.size(); i++) {
                m_positionLookup[m_favoriteScopes[i]] = i;
            }
            Q_ASSERT(m_favoriteScopes.size() == m_positionLookup.size());
            storeFavorites();
            return pos;
        }
    } else {
        int pos = position(scopeId);
        if (pos < 0) {
            m_favoriteScopes.push_back(scopeId);
            pos = m_favoriteScopes.size() - 1;
            m_positionLookup[scopeId] = pos;
        }
        Q_ASSERT(m_favoriteScopes.size() == m_positionLookup.size());
        storeFavorites();
        return pos;
    }

    return -1;
}

void Favorites::moveFavoriteTo(QString const& scopeId, int pos)
{
    int oldPos = position(scopeId);
    if (oldPos >= 0) {
        m_favoriteScopes.move(oldPos, pos);
        auto const range = std::minmax(oldPos, pos);
        for (int i = range.first; i<=range.second; i++) {
            m_positionLookup[m_favoriteScopes[i]] = i;
        }
    } else {
        qWarning() << "Favorites::moveFavoriteTo: no such scope" << scopeId;
    }

    storeFavorites();

    Q_ASSERT(m_favoriteScopes.size() == m_positionLookup.size());
}

QStringList Favorites::getFavorites()
{
    return m_favoriteScopes;
}

bool Favorites::hasScope(QString const& scopeId) const
{
    return m_positionLookup.find(scopeId) != m_positionLookup.end();
}

int Favorites::position(QString const& scopeId) const
{
    auto it = m_positionLookup.find(scopeId);
    if (it != m_positionLookup.end()) {
        return it.value();
    }
    return -1;
}

void Favorites::dashSettingsChanged(QString const &key)
{
    if (key != QLatin1String("favoriteScopes")) {
        return;
    }
    readFavoritesFromGSettings();
    Q_EMIT favoritesChanged();
}

void Favorites::storeFavorites()
{
    if (m_dashSettings) {
        QStringList cannedQueries;
        for (auto const& fav: m_favoriteScopes)
        {
            const QString query = "scope://" + fav;
            cannedQueries.push_back(query);
        }

        QObject::disconnect(m_dashSettings, &QGSettings::changed, this, &Favorites::dashSettingsChanged);
        m_dashSettings->set(QStringLiteral("favoriteScopes"), QVariant(cannedQueries));
        QObject::connect(m_dashSettings, &QGSettings::changed, this, &Favorites::dashSettingsChanged);
    }
}

} // namespace scopes_ng

#include <favorites.moc>
