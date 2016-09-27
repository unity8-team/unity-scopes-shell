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

#ifndef NG_FAVORITES_H
#define NG_FAVORITES_H

#include <QStringList>
#include <QObject>
#include <QPointer>
#include <QMap>

class QGSettings;

namespace scopes_ng
{

class Q_DECL_EXPORT Favorites : public QObject
{
    Q_OBJECT
public:
    Favorites(QObject *parent, QGSettings *dashSettings);
    ~Favorites();

    int setFavorite(QString const& scopeId, bool value);
    void moveFavoriteTo(QString const& scopeId, int pos);
    bool hasScope(QString const& scopeId) const;
    int position(QString const& scopeId) const;
    QStringList getFavorites();
    void storeFavorites();

Q_SIGNALS:
    void favoritesChanged();

private Q_SLOTS:
    void dashSettingsChanged(QString const &key);

private:
    void readFavoritesFromGSettings();

    QPointer<QGSettings> m_dashSettings;
    QStringList m_favoriteScopes;
    QMap<QString, int> m_positionLookup;
};

} // namespace scopes_ng

#endif // NG_SCOPES_H
