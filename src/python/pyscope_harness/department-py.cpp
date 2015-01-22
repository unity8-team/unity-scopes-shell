/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 * Author: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <boost/python.hpp>
#include <scope-harness/results/department.h>
#include <scope-harness/results/child-department.h>

using namespace boost::python;
namespace shr = unity::scopeharness::results;

static object getChildrenDepartments(shr::Department *dep)
{
    list pylist;
    for (auto const child: dep->children())
    {
        pylist.append(child);
    }
    return pylist;
}

static shr::ChildDepartment getChildDepartmentByIndex(shr::Department *dep, int index)
{
    return dep->child(index); // can throw out_of_range
}

/*static bool compareChildDepartments(const shr::ChildDepartment& self, const shr::ChildDepartment& other)
{
    return self == other;
}*/

void export_department()
{
    class_<shr::ChildDepartment>("ChildDepartment",
                                 "Represents a read-only view of a child department.",
                                 no_init)
        .add_property("id", &shr::ChildDepartment::id)
        .add_property("label", &shr::ChildDepartment::label)
        .add_property("has_children", &shr::ChildDepartment::hasChildren)
        .add_property("is_active", &shr::ChildDepartment::isActive)
        //.def("__eq__", compareChildDepartments)
    ;

    class_<shr::Department>("Department",
                            "Represents a read-only view of a department returned by a scope. "
                            "Use id, label, all_label properties to inspect it, and children property or "
                            "child method to inspect child departments (instances of ChildDepartment). "
                            " This class supports __getitem__ call, which acts as a shortcut for child(index) method, "
                            "and responds to __len__ call, so python's len(department) may be used instead of department.size",
                            no_init)
        .add_property("id", &shr::Department::id)
        .add_property("label", &shr::Department::label)
        .add_property("all_label", &shr::Department::allLabel)
        .add_property("parent_id", &shr::Department::parentId)
        .add_property("parent_label", &shr::Department::parentLabel)
        .add_property("is_root", &shr::Department::isRoot)
        .add_property("is_hidden", &shr::Department::isHidden)
        .add_property("size", &shr::Department::size)
        .add_property("children", getChildrenDepartments)
        .def("child", &shr::Department::child, return_value_policy<return_by_value>(), "Get child department by its index (a number)")
        .def("__len__", &shr::Department::size) // respond to python's len(department)
        .def("__getitem__", getChildDepartmentByIndex) // support for [] operator
    ;
}
