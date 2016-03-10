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

#ifndef NG_FILTERGROUPWIDGET_H
#define NG_FILTERGROUPWIDGET_H

#include <unity/shell/scopes/FilterGroupWidgetInterface.h>
#include <unity/shell/scopes/FiltersInterface.h>
#include "filters.h"
#include <unity/scopes/FilterGroup.h>
#include <QScopedPointer>

namespace scopes_ng
{

class Q_DECL_EXPORT FilterGroupWidget : public unity::shell::scopes::FilterGroupWidgetInterface, public FilterUpdateInterface
{
    Q_OBJECT

public:
    FilterGroupWidget(unity::scopes::FilterState::SPtr const& filterState, unity::shell::scopes::FiltersInterface *parent = nullptr);
    QString filterId() const override;
    QString title() const override;
    unity::shell::scopes::FiltersInterface::FilterType filterType() const override;
    QString label() const override;
    unity::shell::scopes::FiltersInterface* filters() const override;
    void update(unity::scopes::FilterBase::SCPtr const& filter) override;
    void update(unity::scopes::FilterState::SPtr const& filterState) override;
    bool isActive() const override;
    QString filterTag() const override;
    void reset() override;

Q_SIGNALS:
    void filterStateChanged();

private:
    QString m_id;
    QString m_title;
    QString m_label;
    QList<QSharedPointer<unity::shell::scopes::FilterBaseInterface>> m_filters;
    std::weak_ptr<unity::scopes::FilterState> m_filterState;
};

} // namespace scopes_ng

#endif
