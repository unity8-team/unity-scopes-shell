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

struct ScopeInfo {
    scopes::ScopeMetadata::SPtr data;
    QString name;

    ScopeInfo(scopes::ScopeMetadata::SPtr const& data_):
        data(data_), name(QString::fromStdString(data->display_name())) {}
};

bool operator<(ScopeInfo const& first, ScopeInfo const& second)
{
    return first.name.compare(second.name, Qt::CaseInsensitive) < 0;
}

void OverviewScope::metadataChanged()
{
    OverviewCategories* categories = qobject_cast<OverviewCategories*>(m_categories.data());
    if (!categories) {
        qWarning("Unable to cast m_categories to OverviewCategories");
        return;
    }

    QMap<QString, scopes::ScopeMetadata::SPtr> allMetadata = m_scopesInstance->getAllMetadata();

    QMap<QString, QString> scopeIdToName;
    for (auto m: allMetadata)
    {
        scopeIdToName[QString::fromStdString(m->scope_id())] = QString::fromStdString(m->display_name());
    }

    QList<scopes::ScopeMetadata::SPtr> favorites;
    Q_FOREACH(QString id, m_scopesInstance->getFavoriteIds()) {
        auto it = allMetadata.find(id);
        if (it != allMetadata.end()) {
            favorites.append(it.value());
            allMetadata.erase(it);
        }
    }

    QList<ScopeInfo> scopes;
    Q_FOREACH(scopes::ScopeMetadata::SPtr const& metadata, allMetadata.values()) {
        if (metadata->invisible()) continue;
        scopes.append(ScopeInfo(metadata));
    }
    qSort(scopes.begin(), scopes.end());

    QList<scopes::ScopeMetadata::SPtr> allScopes;
    Q_FOREACH(ScopeInfo const& info, scopes) {
        allScopes << info.data;
    }

    // FIXME: filter invisible scopes?
    categories->setOtherScopes(allScopes, scopeIdToName);
    categories->setFavoriteScopes(favorites, scopeIdToName);

    // Metadata has changed, invalidate the search results
    invalidateResults();
}

QString OverviewScope::id() const
{
    return QString("scopes");
}

scopes::ScopeProxy OverviewScope::proxy_for_result(scopes::Result::SPtr const& result) const
{
    try {
        return result->target_scope_proxy();
    } catch (...) {
        // our fake results don't have a proxy associated, return the default one
        return proxy();
    }
}

void OverviewScope::updateFavorites(const QStringList& favorites)
{
    QList<scopes::ScopeMetadata::SPtr> favs;
    auto allMetadata = m_scopesInstance->getAllMetadata();

    QMap<QString, QString> scopeIdToName;
    for (auto m: allMetadata)
    {
        scopeIdToName[QString::fromStdString(m->scope_id())] = QString::fromStdString(m->display_name());
    }

    for (auto const id: favorites)
    {
        auto it = allMetadata.find(id);
        if (it != allMetadata.end()) {
            favs.append(it.value());
            allMetadata.erase(it);
        }
    }

    OverviewCategories* categories = qobject_cast<OverviewCategories*>(m_categories.data());
    if (!categories) {
        qWarning("Unable to cast m_categories to OverviewCategories");
        return;
    }

    QList<ScopeInfo> scopes;
    Q_FOREACH(scopes::ScopeMetadata::SPtr const& metadata, allMetadata.values()) {
        if (metadata->invisible()) continue;
        scopes.append(ScopeInfo(metadata));
    }
    qSort(scopes.begin(), scopes.end());

    QList<scopes::ScopeMetadata::SPtr> otherScopes;
    Q_FOREACH(ScopeInfo const& info, scopes) {
        otherScopes << info.data;
    }

    categories->updateOtherScopes(otherScopes, scopeIdToName);
    categories->updateFavoriteScopes(favs, scopeIdToName);
}

void OverviewScope::dispatchSearch()
{
    OverviewCategories* categories = qobject_cast<OverviewCategories*>(m_categories.data());
    if (!categories) {
        qWarning("Unable to cast m_categories to OverviewCategories");
        return;
    }

    if (searchQuery().isEmpty()) {
        setSearchInProgress(true);
        invalidateLastSearch();
        categories->setSurfacingMode(true);
        setSearchInProgress(false);
    } else {
        categories->setSurfacingMode(false);
        Scope::dispatchSearch();
    }
}

} // namespace scopes_ng
