/*
 * Copyright (C) 2013 Canonical, Ltd.
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
#include "scope.h"

// local
//#include "categories.h"
//#include "preview.h"

// Qt
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QtGui/QDesktopServices>
#include <QQmlEngine>

#include <libintl.h>
#include <glib.h>

namespace scopes_ng
{

Scope::Scope(QObject *parent) : QObject(parent)
    , m_formFactor("phone")
    , m_isActive(false)
    , m_searchInProgress(false)
{
    //m_categories.reset(new Categories(this));
}

QString Scope::id() const
{
    // FIXME: get from somewhere
    return QString::fromStdString("foo");
}

QString Scope::name() const
{
    // FIXME: get from scope config
    return QString::fromStdString("Foo");
}

QString Scope::iconHint() const
{
    // FIXME: get from scope config
    return QString::fromStdString("");
}

QString Scope::description() const
{
    // FIXME: get from scope config
    return QString::fromStdString("");
}

QString Scope::searchHint() const
{
    // FIXME: get from scope config
    return QString::fromStdString("Search");
}

bool Scope::searchInProgress() const
{
    return m_searchInProgress;
}

bool Scope::visible() const
{
    // FIXME: get from scope config
    return true;
}

QString Scope::shortcut() const
{
    // FIXME: get from scope config
    return QString::fromStdString("");
}

bool Scope::connected() const
{
    // FIXME: get from scope proxy?
    return false;
}

/*
Categories* Scope::categories() const
{
    return m_categories.get();
}

Filters* Scope::filters() const
{
    return m_filters.get();
}
*/

QString Scope::searchQuery() const
{
    return m_searchQuery;
}

QString Scope::noResultsHint() const
{
    return m_noResultsHint;
}

QString Scope::formFactor() const
{
    return m_formFactor;
}

bool Scope::isActive() const
{
    return m_isActive;
}

void Scope::setSearchQuery(const QString& search_query)
{
    /* Checking for m_searchQuery.isNull() which returns true only when the string
       has never been set is necessary because when search_query is the empty
       string ("") and m_searchQuery is the null string,
       search_query != m_searchQuery is still true.
    */

    if (m_searchQuery.isNull() || search_query != m_searchQuery) {
        m_searchQuery = search_query;
        // FIXME: dispatch search to the proxy
        Q_EMIT searchQueryChanged();
        if (!m_searchInProgress) {
            m_searchInProgress = true;
            Q_EMIT searchInProgressChanged();
        }
    }
}

void Scope::setNoResultsHint(const QString& hint) {
    if (hint != m_noResultsHint) {
        m_noResultsHint = hint;
        Q_EMIT noResultsHintChanged();
    }
}

void Scope::setFormFactor(const QString& form_factor) {
    if (form_factor != m_formFactor) {
        m_formFactor = form_factor;
        // FIXME: force new search
        Q_EMIT formFactorChanged();
    }
}

void Scope::setActive(const bool active) {
    if (active != m_isActive) {
        m_isActive = active;
        Q_EMIT isActiveChanged(m_isActive);
    }
}

// FIXME: Change to use row index.
void Scope::activate(const QVariant &uri, const QVariant &icon_hint, const QVariant &category,
                     const QVariant &result_type, const QVariant &mimetype, const QVariant &title,
                     const QVariant &comment, const QVariant &dnd_uri, const QVariant &metadata)
{
}

// FIXME: Change to use row index.
void Scope::preview(const QVariant &uri, const QVariant &icon_hint, const QVariant &category,
             const QVariant &result_type, const QVariant &mimetype, const QVariant &title,
             const QVariant &comment, const QVariant &dnd_uri, const QVariant &metadata)
{
    // FIXME: handle overridden results
}

void Scope::cancelActivation()
{
}

} // namespace scopes_ng
