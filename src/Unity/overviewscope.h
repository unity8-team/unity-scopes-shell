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

#ifndef NG_OVERVIEW_SCOPE_H
#define NG_OVERVIEW_SCOPE_H

#include "scope.h"

namespace scopes_ng
{

class Q_DECL_EXPORT OverviewScope : public scopes_ng::Scope
{
    Q_OBJECT

public:
    explicit OverviewScope(QObject *parent = 0);
    virtual ~OverviewScope();

    /* getters */
    QString id() const override;
    bool visible() const override;

    void dispatchSearch() override;

private Q_SLOTS:
    void metadataChanged();

private:
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::OverviewScope*)

#endif // NG_OVERVIEW_SCOPE_H
