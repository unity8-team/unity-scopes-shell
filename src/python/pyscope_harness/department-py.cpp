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
    return pylist; //TODO test if works
}

void export_department()
{
    class_<shr::ChildDepartment>("ChildDepartment", no_init)
        .add_property("id", &shr::ChildDepartment::id)
        .add_property("label", &shr::ChildDepartment::label)
        .add_property("has_children", &shr::ChildDepartment::hasChildren)
        .add_property("is_active", &shr::ChildDepartment::isActive)
    ;

    class_<shr::Department>("Department", no_init)
        .add_property("id", &shr::Department::id)
        .add_property("label", &shr::Department::label)
        .add_property("all_label", &shr::Department::allLabel)
        .add_property("parent_id", &shr::Department::parentId)
        .add_property("parent_label", &shr::Department::parentLabel)
        .add_property("is_root", &shr::Department::isRoot)
        .add_property("is_hidden", &shr::Department::isHidden)
        .add_property("size", &shr::Department::size)
        .add_property("children", getChildrenDepartments)
        .def("child", &shr::Department::child)
        .def("__len__", &shr::Department::size) // respond to python's len(department)
        // TODO: getitem
    ;
}
