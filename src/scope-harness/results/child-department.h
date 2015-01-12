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

#include <memory>
#include <string>
#include <vector>

#include <QtGlobal>

namespace unity
{
namespace scopeharness
{
namespace internal
{
struct ChildDepartmentArguments;
}
namespace results
{
class Department;

class Q_DECL_EXPORT ChildDepartment
{
public:
    typedef std::vector<ChildDepartment> List;

    ChildDepartment(const ChildDepartment& other);

    ChildDepartment(ChildDepartment&& other);

    ~ChildDepartment();

    ChildDepartment& operator=(const ChildDepartment& other);

    ChildDepartment& operator=(ChildDepartment&& other);

    std::string id() const;

    std::string label() const;

    bool hasChildren() const;

    bool isActive() const;

protected:
    friend Department;

    ChildDepartment(const internal::ChildDepartmentArguments& arguments);

    struct Priv;

    std::shared_ptr<Priv> p;
};

}
}
}
