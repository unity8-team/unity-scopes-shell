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

#include <unity/shell/scopes/FiltersInterface.h>
#include <unity/shell/scopes/ScopeInterface.h>
#include <unity/scopes/FilterBase.h>
#include <unity/scopes/FilterState.h>
#include <unity/shell/scopes/FilterBaseInterface.h>
#include "modelupdate.h"

#include <QList>
#include <QSharedPointer>

namespace scopes_ng
{

class Q_DECL_EXPORT Filters :
    public ModelUpdate<unity::shell::scopes::FiltersInterface,
        QList<unity::scopes::FilterBase::SCPtr>,
        QList<QSharedPointer<unity::shell::scopes::FilterBaseInterface>>>
{
    Q_OBJECT

public:
    explicit Filters(unity::shell::scopes::ScopeInterface *parent = nullptr);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void clear();
    void update(QList<unity::scopes::FilterBase::SCPtr> const& filters, unity::scopes::FilterState const& filterState);

    unity::scopes::FilterState filterState() const;

Q_SIGNALS:
    void filterStateChanged();

private:
    static unity::shell::scopes::FiltersInterface::FilterType getFilterType(unity::scopes::FilterBase::SCPtr const& filter);
    QSharedPointer<unity::shell::scopes::FilterBaseInterface> createFilterObject(unity::scopes::FilterBase::SCPtr const& filter);
    QList<QSharedPointer<unity::shell::scopes::FilterBaseInterface>> m_filters;
    unity::scopes::FilterState::SPtr m_filterState;
};

} // namespace scopes_ng

#endif
