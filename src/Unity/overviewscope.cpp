/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *  Michal Hruby <michal.hruby@canonical.com>
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

// Self
#include "overviewscope.h"

// local
#include "overviewcategories.h"
#include "scopes.h"
#include "utils.h"

// Qt
#include <QScopedPointer>

namespace scopes_ng
{

using namespace unity;

OverviewScope::OverviewScope(QObject *parent) : scopes_ng::Scope(parent)
{
    m_categories.reset(new OverviewCategories(this));

    QObject::connect(m_scopesInstance.data(), &Scopes::metadataRefreshed, this, &OverviewScope::metadataChanged);
}

OverviewScope::~OverviewScope()
{
}

void OverviewScope::metadataChanged()
{
    OverviewCategories* categories = qobject_cast<OverviewCategories*>(m_categories.data());
    if (!categories) {
        qWarning("Unable to cast m_categories to OverviewCategories");
        return;
    }

    QMap<QString, scopes::ScopeMetadata::SPtr> allMetadata = m_scopesInstance->getAllMetadata();
    QList<scopes::ScopeMetadata::SPtr> favourites;
    Q_FOREACH(QString id, m_scopesInstance->getFavoriteIds()) {
        auto it = allMetadata.find(id);
        if (it != allMetadata.end()) {
            favourites.append(it.value());
        }
    }

    // FIXME: filter invisible scopes?
    categories->setAllScopes(allMetadata.values());
    categories->setFavouriteScopes(favourites);
}

QString OverviewScope::id() const
{
    return QString("scopesOverview");
}

bool OverviewScope::visible() const
{
    return false;
}

void OverviewScope::dispatchSearch()
{
    OverviewCategories* categories = qobject_cast<OverviewCategories*>(m_categories.data());
    if (!categories) {
        qWarning("Unable to cast m_categories to OverviewCategories");
        return;
    }

    QString search_query(searchQuery());

    categories->setSurfacingMode(search_query.isEmpty());

    if (!search_query.isEmpty()) {
        Scope::dispatchSearch();
    } else {
        invalidateLastSearch();
    }
}

} // namespace scopes_ng
