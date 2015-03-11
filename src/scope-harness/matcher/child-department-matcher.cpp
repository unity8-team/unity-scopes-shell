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
#include <scope-harness/results/child-department.h>

#include <boost/optional.hpp>

using namespace std;
using namespace boost;

namespace unity
{
namespace scopeharness
{
namespace matcher
{
namespace
{
static void check_string(MatchResult& matchResult, const results::ChildDepartment& department,
             const string& name, const string& actualValue,
             const string& expectedValue)
{
    if (actualValue != expectedValue)
    {
        matchResult.failure(
                "Child department with ID '" + department.id() + "' has '" + name
                        + "' == '" + actualValue + "' but expected '"
                        + expectedValue + "'");
    }
}

static void check_bool(MatchResult& matchResult, const results::ChildDepartment& department,
             const string& name, bool actualValue,
             bool expectedValue)
{
    if (actualValue != expectedValue)
    {
        matchResult.failure(
                "Child department with ID '" + department.id() + "' has '" + name
                        + "' == " + (actualValue ? "true" : "false") + " but expected "
                        + (expectedValue ? "true" : "false"));
    }
}
}

struct ChildDepartmentMatcher::Priv
{
    string m_id;

    optional<string> m_label;

    optional<bool> m_hasChildren;

    optional<bool> m_isActive;
};

ChildDepartmentMatcher::ChildDepartmentMatcher(const string& id) :
        p(new Priv)
{
    p->m_id = id;
}

ChildDepartmentMatcher::~ChildDepartmentMatcher()
{
}

ChildDepartmentMatcher::ChildDepartmentMatcher(const ChildDepartmentMatcher& other) :
        p(new Priv)
{
    *this = other;
}

ChildDepartmentMatcher::ChildDepartmentMatcher(ChildDepartmentMatcher&& other)
{
    *this = move(other);
}

ChildDepartmentMatcher& ChildDepartmentMatcher::operator=(const ChildDepartmentMatcher& other)
{
    p->m_id = other.p->m_id;
    p->m_label = other.p->m_label;
    p->m_hasChildren = other.p->m_hasChildren;
    p->m_isActive = other.p->m_isActive;
    return *this;
}

ChildDepartmentMatcher& ChildDepartmentMatcher::operator=(ChildDepartmentMatcher&& other)
{
    p = move(other.p);
    return *this;
}

string ChildDepartmentMatcher::getId() const
{
    return p->m_id;
}

ChildDepartmentMatcher& ChildDepartmentMatcher::label(const std::string& label)
{
    p->m_label = label;
    return *this;
}

ChildDepartmentMatcher& ChildDepartmentMatcher::hasChildren(bool hasChildren)
{
    p->m_hasChildren = hasChildren;
    return *this;
}

ChildDepartmentMatcher& ChildDepartmentMatcher::isActive(bool isActive)
{
    p->m_isActive = isActive;
    return *this;
}

MatchResult ChildDepartmentMatcher::match(const results::ChildDepartment& childDepartment) const
{
    MatchResult matchResult;
    match(matchResult, childDepartment);
    return matchResult;
}

void ChildDepartmentMatcher::match(MatchResult& matchResult, const results::ChildDepartment& childDepartment) const
{
    check_string(matchResult, childDepartment, "id", childDepartment.id(), p->m_id);

    if (p->m_label)
    {
        check_string(matchResult, childDepartment, "label", childDepartment.label(), p->m_label.get());
    }

    if (p->m_hasChildren)
    {
        check_bool(matchResult, childDepartment, "has children", childDepartment.hasChildren(), p->m_hasChildren.get());
    }

    if (p->m_isActive)
    {
        check_bool(matchResult, childDepartment, "is active", childDepartment.isActive(), p->m_isActive.get());
    }
}

}
}
}
