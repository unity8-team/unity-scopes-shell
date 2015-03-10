/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#ifndef NG_FILTERS_H
#define NG_FILTERS_H

#include "FiltersInterface.h"
#include <unity/shell/scopes/ScopeInterface.h>
#include <unity/scopes/FilterBase.h>
#include <unity/scopes/FilterState.h>
#include "FilterBaseInterface.h"

#include <QList>

namespace scopes_ng
{

class Q_DECL_EXPORT Filters : public unity::shell::scopes::FiltersInterface
{
    Q_OBJECT

public:
    explicit Filters(unity::shell::scopes::ScopeInterface *parent = nullptr);
    int rowCount(const QModelIndex& parent) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void clear();
    void update(QList<unity::scopes::FilterBase::SCPtr> const& filters, unity::scopes::FilterState const& filterState);

Q_SIGNALS:
    void filterStateChanged();

private:
    unity::shell::scopes::FilterBaseInterface* createFilterObject(unity::scopes::FilterBase::SCPtr const& filter);
    QList<unity::shell::scopes::FilterBaseInterface*> m_filters;
    unity::scopes::FilterState::SPtr m_filterState;
};

} // namespace scopes_ng

#endif
