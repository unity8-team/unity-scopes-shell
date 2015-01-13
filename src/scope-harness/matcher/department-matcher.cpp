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

#include <scope-harness/results/department.h>
#include <scope-harness/matcher/department-matcher.h>

#include <boost/optional.hpp>
#include <unordered_map>

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
static void check_string(MatchResult& matchResult, const results::Department& department,
             const string& name, const string& actualValue,
             const string& expectedValue)
{
    if (actualValue != expectedValue)
    {
        matchResult.failure(
                "Department with ID '" + department.id() + "' has '" + name
                        + "' == '" + actualValue + "' but expected '"
                        + expectedValue + "'");
    }
}

static void check_bool(MatchResult& matchResult, const results::Department& department,
             const string& name, bool actualValue,
             bool expectedValue)
{
    if (actualValue != expectedValue)
    {
        matchResult.failure(
                "Department with ID '" + department.id() + "' has '" + name
                        + "' == " + (actualValue ? "true" : "false") + " but expected "
                        + (expectedValue ? "true" : "false"));
    }
}
}

struct DepartmentMatcher::Priv
{
    void all(MatchResult& matchResult, const results::Department& department)
    {
        if (department.size() != m_children.size())
        {
            matchResult.failure(
                    "Department contained " + to_string(department.size())
                            + " children, expected " + to_string(m_children.size()));
            return;
        }

        for (size_t row = 0; row < m_children.size(); ++row)
        {
            const auto& expectedChild = m_children[row];
            auto actualChild = department.child(row);
            expectedChild.match(matchResult, actualChild);
        }
    }

    void byId(MatchResult& matchResult, const results::Department& department)
    {
        unordered_map<string, results::ChildDepartment> childDepartmentsById;
        for (size_t row = 0; row < department.size(); ++row)
        {
            auto child = department.child(row);
            childDepartmentsById.insert({child.id(), child});
        }

        for (const auto& expectedChild : m_children)
        {
            auto it = childDepartmentsById.find(expectedChild.getId());
            if (it == childDepartmentsById.end())
            {
                matchResult.failure(
                        "Child department with ID " + expectedChild.getId()
                                + " could not be found");
            }
            else
            {
                expectedChild.match(matchResult, it->second);
            }
        }
    }

    void startsWith(MatchResult& matchResult, const results::Department& department)
    {
        matchResult.failure("not implemented - startsWith");
        if (department.size() < m_children.size())
        {
            matchResult.failure(
                    "Department contained " + to_string(department.size())
                            + " expected at least " + to_string(m_children.size()));
            return;
        }

        for (size_t row = 0; row < m_children.size(); ++row)
        {
            const auto& expectedChild = m_children[row];
            auto actualChild = department.child(row);
            expectedChild.match(matchResult, actualChild);
        }
    }

    optional<string> m_id;

    optional<string> m_label;

    optional<string> m_allLabel;

    optional<string> m_parentId;

    optional<string> m_parentLabel;

    optional<bool> m_isRoot;

    optional<bool> m_isHidden;

    optional<size_t> m_hasExactly;

    optional<size_t> m_hasAtLeast;

    Mode m_mode = Mode::all;

    vector<ChildDepartmentMatcher> m_children;
};

DepartmentMatcher::DepartmentMatcher() :
        p(new Priv)
{
}

DepartmentMatcher::~DepartmentMatcher()
{
}

DepartmentMatcher& DepartmentMatcher::mode(Mode mode)
{
    p->m_mode = mode;
    return *this;
}

DepartmentMatcher& DepartmentMatcher::hasExactly(size_t childCount)
{
    p->m_hasExactly = childCount;
    return *this;
}

DepartmentMatcher& DepartmentMatcher::hasAtLeast(size_t childCount)
{
    p->m_hasAtLeast = childCount;
    return *this;
}

DepartmentMatcher& DepartmentMatcher::id(const string& id)
{
    p->m_id = id;
    return *this;
}

DepartmentMatcher& DepartmentMatcher::label(const string& label)
{
    p->m_label = label;
    return *this;
}

DepartmentMatcher& DepartmentMatcher::allLabel(const string& allLabel)
{
    p->m_allLabel = allLabel;
    return *this;
}

DepartmentMatcher& DepartmentMatcher::parentId(const string& parentId)
{
    p->m_parentId = parentId;
    return *this;
}

DepartmentMatcher& DepartmentMatcher::parentLabel(const string& parentLabel)
{
    p->m_parentLabel = parentLabel;
    return *this;
}

DepartmentMatcher& DepartmentMatcher::isRoot(bool isRoot)
{
    p->m_isRoot = isRoot;
    return *this;
}

DepartmentMatcher& DepartmentMatcher::isHidden(bool isHidden)
{
    p->m_isHidden = isHidden;
    return *this;
}

DepartmentMatcher& DepartmentMatcher::child(const ChildDepartmentMatcher& child)
{
    p->m_children.emplace_back(child);
    return *this;
}

DepartmentMatcher& DepartmentMatcher::child(ChildDepartmentMatcher&& child)
{
    p->m_children.emplace_back(child);
    return *this;
}

MatchResult DepartmentMatcher::match(const results::Department& department) const
{
    MatchResult matchResult;
    match(matchResult, department);
    return matchResult;
}

void DepartmentMatcher::match(MatchResult& matchResult, const results::Department& department) const
{
    if (p->m_id)
    {
        check_string(matchResult, department, "id", department.id(), p->m_id.get());
    }

    if (p->m_label)
    {
        check_string(matchResult, department, "label", department.label(), p->m_label.get());
    }

    if (p->m_allLabel)
    {
        check_string(matchResult, department, "all label", department.allLabel(), p->m_allLabel.get());
    }

    if (p->m_parentId)
    {
        check_string(matchResult, department, "parent ID", department.parentId(), p->m_parentId.get());
    }

    if (p->m_parentLabel)
    {
        check_string(matchResult, department, "parent label", department.parentLabel(), p->m_parentLabel.get());
    }

    if (p->m_isRoot)
    {
        check_bool(matchResult, department, "is root", department.isRoot(), p->m_isRoot.get());
    }

    if (p->m_isHidden)
    {
        check_bool(matchResult, department, "is hidden", department.isHidden(), p->m_isHidden.get());
    }

    if (p->m_hasExactly && department.size() != p->m_hasExactly.get())
    {
        matchResult.failure(
                "Expected exactly " + to_string(p->m_hasExactly.get())
                        + " child departments");
    }

    if (p->m_hasAtLeast && department.size() < p->m_hasAtLeast.get())
    {
        matchResult.failure(
                "Expected at least " + to_string(p->m_hasAtLeast.get())
                        + " child departments");
    }

    if (!p->m_children.empty())
    {
        switch (p->m_mode)
        {
            case Mode::all:
                p->all(matchResult, department);
                break;
            case Mode::by_id:
                p->byId(matchResult, department);
                break;
            case Mode::starts_with:
                p->startsWith(matchResult, department);
                break;
        }
    }
}

}
}
}
