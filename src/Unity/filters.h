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
#include <QTimer>

namespace scopes_ng
{

struct FilterWrapper
{
    UNITY_DEFINES_PTRS(FilterWrapper);

    QList<unity::scopes::FilterBase::SCPtr> filters;
    std::string id() const;
    bool isGroup() const;
};

class FilterUpdateInterface
{
    public:
        // Apply potential updates of filter definition coming from scope.
        virtual void update(unity::scopes::FilterBase::SCPtr const& filter) = 0;
        virtual void update(FilterWrapper::SCPtr const& filterWrapper);

        // Apply filter state change (e.g. after user executed a canned query).
        virtual void update(unity::scopes::FilterState::SPtr const& filterState) = 0;

        // Check if the filter is active (i.e. has non-default values set).
        virtual bool isActive() const = 0;

        // Reset filter to defaults.
        virtual void reset() = 0;

        virtual ~FilterUpdateInterface() {}
};

class Q_DECL_EXPORT Filters :
    public ModelUpdate<unity::shell::scopes::FiltersInterface,
        QList<FilterWrapper::SCPtr>,
        QList<QSharedPointer<unity::shell::scopes::FilterBaseInterface>>>
{
    Q_OBJECT

public:
    explicit Filters(unity::scopes::FilterState const& filterState, unity::shell::scopes::ScopeInterface *parent = nullptr);
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    void update(QList<unity::scopes::FilterBase::SCPtr> const& filters, bool containsDepartments = false);
    void update(unity::scopes::FilterState const& filterState);

    unity::scopes::FilterState filterState() const;
    QSharedPointer<unity::shell::scopes::FilterBaseInterface> primaryFilter() const;
    int activeFiltersCount() const;

public Q_SLOTS:
    void clear();
    void reset();

private Q_SLOTS:
    void onFilterStateChanged();
    void delayedFilterStateChange();

Q_SIGNALS:
    void filterStateChanged();
    void primaryFilterChanged();

private:
    static QList<FilterWrapper::SCPtr> preprocessFilters(QList<unity::scopes::FilterBase::SCPtr> const &filters);
    static unity::shell::scopes::FiltersInterface::FilterType getFilterType(FilterWrapper::SCPtr const& filterWrapper);
    static unity::shell::scopes::FiltersInterface::FilterType getFilterType(unity::scopes::FilterBase::SCPtr const& filter);
    QSharedPointer<unity::shell::scopes::FilterBaseInterface> createFilterObject(unity::scopes::FilterBase::SCPtr const& filter);
    QSharedPointer<unity::shell::scopes::FilterBaseInterface> createFilterObject(FilterWrapper::SCPtr const& filterWrapper);
    QList<QSharedPointer<unity::shell::scopes::FilterBaseInterface>> m_filters;
    QSharedPointer<unity::shell::scopes::FilterBaseInterface> m_primaryFilter;
    unity::scopes::FilterState::SPtr m_filterState;
    QTimer m_filterStateChangeTimer;
};

} // namespace scopes_ng

#endif
