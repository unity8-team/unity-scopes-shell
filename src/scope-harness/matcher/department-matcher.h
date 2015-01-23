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

#include <scope-harness/matcher/child-department-matcher.h>

#pragma once

namespace unity
{
namespace scopeharness
{
namespace results
{
class Department;
}
namespace matcher
{

class Q_DECL_EXPORT DepartmentMatcher
{
public:
    enum class Mode
    {
        all, by_id, starts_with
    };

    DepartmentMatcher();

    DepartmentMatcher(const DepartmentMatcher& other) = delete;

    DepartmentMatcher(DepartmentMatcher&& other) = delete;

    ~DepartmentMatcher();

    DepartmentMatcher& operator=(const DepartmentMatcher& other) = delete;

    DepartmentMatcher& operator=(DepartmentMatcher&& other) = delete;

    DepartmentMatcher& mode(Mode mode);

    DepartmentMatcher& hasExactly(std::size_t childCount);

    DepartmentMatcher& hasAtLeast(std::size_t childCount);

    DepartmentMatcher& id(const std::string& id);

    DepartmentMatcher& label(const std::string& label);

    DepartmentMatcher& allLabel(const std::string& allLabel);

    DepartmentMatcher& parentId(const std::string& parentId);

    DepartmentMatcher& parentLabel(const std::string& parentLabel);

    DepartmentMatcher& isRoot(bool isRoot);

    DepartmentMatcher& isHidden(bool isHidden);

    DepartmentMatcher& child(const ChildDepartmentMatcher& child);

    DepartmentMatcher& child(ChildDepartmentMatcher&& child);

    MatchResult match(const results::Department& department) const;

    void match(MatchResult& matchResult, const results::Department& department) const;

protected:
    struct Priv;

    std::shared_ptr<Priv> p;
};

}
}
}
