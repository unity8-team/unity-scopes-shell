/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This library is free software
{
} you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY
{
} without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <scope-harness/internal/child-department-arguments.h>
#include <scope-harness/internal/department-arguments.h>
#include <scope-harness/results/department.h>

#include <unity/shell/scopes/NavigationInterface.h>

using namespace std;

namespace ss = unity::shell::scopes;

namespace unity
{
namespace scopeharness
{
namespace results
{

struct Department::_Priv
{
    QSharedPointer<ss::NavigationInterface> m_navigationModel;

    ChildDepartment::List m_children;
};

Department::Department(const internal::DepartmentArguments& arguments) :
        p(new _Priv)
{
    p->m_navigationModel = arguments.navigationModel;

    for (int row = 0; row < p->m_navigationModel->rowCount(); ++row)
    {
        auto index = p->m_navigationModel->index(row);
        p->m_children.emplace_back(
                ChildDepartment(internal::ChildDepartmentArguments
                { p->m_navigationModel, index }));
    }
}

Department::~Department()
{
}

Department::Department(const Department& other) :
    p(new _Priv)
{
    *this = other;
}

Department::Department(Department&& other)
{
    *this = std::move(other);
}

Department& Department::operator=(const Department& other)
{
    p->m_navigationModel = other.p->m_navigationModel;
    p->m_children = other.p->m_children;
    return *this;
}

Department& Department::operator=(Department&& other)
{
    p = std::move(other.p);
    return *this;
}

string Department::id() const
{
    return p->m_navigationModel->navigationId().toStdString();
}

string Department::label() const
{
    return p->m_navigationModel->label().toStdString();
}

string Department::allLabel() const
{
    return p->m_navigationModel->allLabel().toStdString();
}

string Department::parentId() const
{
    return p->m_navigationModel->parentNavigationId().toStdString();
}

string Department::parentLabel() const
{
    return p->m_navigationModel->parentLabel().toStdString();
}

bool Department::loaded() const
{
    return true;
}

bool Department::isRoot() const
{
    return p->m_navigationModel->isRoot();
}

bool Department::isHidden() const
{
    return p->m_navigationModel->hidden();
}

size_t Department::size() const
{
    return p->m_children.size();
}

ChildDepartment::List Department::children() const
{
    return p->m_children;
}

ChildDepartment Department::child(size_t index) const
{
    return p->m_children.at(index);
}

}
}
}
