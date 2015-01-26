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

#include <scope-harness/results/child-department.h>

namespace unity
{
namespace scopeharness
{
namespace internal
{
struct DepartmentArguments;
}
namespace view
{
class ResultsView;
}
namespace results
{

class Q_DECL_EXPORT Department final
{
public:
    Department(const Department& other);

    Department(Department&& other);

    ~Department();

    Department& operator=(const Department& other);

    Department& operator=(Department&& other);

    std::string id() const;

    std::string label() const;

    std::string allLabel() const;

    std::string parentId() const;

    std::string parentLabel() const;

    bool loaded() const;

    bool isRoot() const;

    bool isHidden() const;

    std::size_t size() const;

    ChildDepartment::List children() const;

    ChildDepartment child(std::size_t index) const;

protected:
    friend view::ResultsView;

    Department(const internal::DepartmentArguments& arguments);

    struct Priv;

    std::shared_ptr<Priv> p;
};

}
}
}
