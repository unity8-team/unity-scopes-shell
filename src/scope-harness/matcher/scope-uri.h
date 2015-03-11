/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#pragma once

#include <QtGlobal>

#include <memory>
#include <string>

namespace unity
{
namespace scopeharness
{
namespace matcher
{

class Q_DECL_EXPORT ScopeUri
{
public:
    ScopeUri(const std::string& id);

    ScopeUri(const ScopeUri& other);

    ScopeUri(ScopeUri&& other);

    ~ScopeUri();

    ScopeUri& operator=(const ScopeUri& other);

    ScopeUri& operator=(ScopeUri&& other);

    ScopeUri& department(const std::string& departmentId);

    ScopeUri& query(const std::string& queryString);

    std::string toString() const;

protected:
    struct Priv;

    std::shared_ptr<Priv> p;
};

}
}
}
