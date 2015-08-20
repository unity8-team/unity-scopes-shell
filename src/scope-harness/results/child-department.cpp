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
#include <scope-harness/results/child-department.h>

#include <unity/shell/scopes/NavigationInterface.h>

using namespace std;

namespace ss = unity::shell::scopes;

namespace unity
{
namespace scopeharness
{
namespace results
{

struct ChildDepartment::_Priv
{
    QSharedPointer<ss::NavigationInterface> m_navigationModel;

    QModelIndex m_index;
};

ChildDepartment::ChildDepartment(const internal::ChildDepartmentArguments& arguments) :
        p(new _Priv)
{
    p->m_navigationModel = arguments.navigationModel;
    p->m_index = arguments.index;
}

ChildDepartment::~ChildDepartment()
{
}

ChildDepartment::ChildDepartment(const ChildDepartment& other) :
    p(new _Priv)
{
    *this = other;
}

ChildDepartment::ChildDepartment(ChildDepartment&& other)
{
    *this = std::move(other);
}

ChildDepartment& ChildDepartment::operator=(const ChildDepartment& other)
{
    p->m_navigationModel = other.p->m_navigationModel;
    p->m_index = other.p->m_index;
    return *this;
}

ChildDepartment& ChildDepartment::operator=(ChildDepartment&& other)
{
    p = std::move(other.p);
    return *this;
}


string ChildDepartment::id() const
{
    return p->m_navigationModel->data(p->m_index,
                                      ss::NavigationInterface::RoleNavigationId).toString().toStdString();
}

string ChildDepartment::label() const
{
    return p->m_navigationModel->data(p->m_index,
                                      ss::NavigationInterface::RoleLabel).toString().toStdString();
}

bool ChildDepartment::hasChildren() const
{
    return p->m_navigationModel->data(p->m_index,
                                      ss::NavigationInterface::RoleHasChildren).toBool();
}

bool ChildDepartment::isActive() const
{
    return p->m_navigationModel->data(p->m_index,
                                      ss::NavigationInterface::RoleIsActive).toBool();
}

}
}
}
