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

#include <scope-harness/matcher/match-result.h>

namespace unity
{
namespace scopeharness
{
namespace results
{
class ChildDepartment;
}
namespace matcher
{

class Q_DECL_EXPORT ChildDepartmentMatcher final
{
public:
    ChildDepartmentMatcher(const std::string& id);

    ChildDepartmentMatcher(const ChildDepartmentMatcher& other);

    ChildDepartmentMatcher(ChildDepartmentMatcher&& other);

    ~ChildDepartmentMatcher();

    ChildDepartmentMatcher& operator=(const ChildDepartmentMatcher& other);

    ChildDepartmentMatcher& operator=(ChildDepartmentMatcher&& other);

    std::string getId() const;

    ChildDepartmentMatcher& label(const std::string& label);

    ChildDepartmentMatcher& hasChildren(bool hasChildren);

    ChildDepartmentMatcher& isActive(bool isActive);

    MatchResult match(const results::ChildDepartment& childDepartment) const;

    void match(MatchResult& matchResult, const results::ChildDepartment& childDepartment) const;

protected:
    struct Priv;

    std::shared_ptr<Priv> p;
};

}
}
}
