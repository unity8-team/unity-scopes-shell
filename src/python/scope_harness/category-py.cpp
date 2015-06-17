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
 * Author: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <boost/python.hpp>
#include <scope-harness/results/category.h>

using namespace boost::python;
namespace shr = unity::scopeharness::results;

static object getResultsList(const shr::Category& cat)
{
    list pylist;
    for (auto const res: cat.results())
    {
        pylist.append(res);
    }
    return pylist;
}

void export_category()
{
    shr::Result (shr::Category::*result_by_uri)(const std::string&) const = &shr::Category::result;
    shr::Result (shr::Category::*result_by_index)(std::size_t) const = &shr::Category::result;

    class_<shr::Category>("Category", "Represents a read-only view of a category returned by scope",
                          no_init)
        .add_property("id", &shr::Category::id)
        .add_property("title", &shr::Category::title)
        .add_property("icon", &shr::Category::icon)
        .add_property("header_link", &shr::Category::headerLink)
        .add_property("renderer", &shr::Category::renderer)
        .add_property("components", &shr::Category::components)
        .add_property("results", &getResultsList)
        .add_property("empty", &shr::Category::empty)
        .def("result", result_by_uri, return_value_policy<return_by_value>(),
            "Get a Result instance by its uri.\n\n"
            ":param arg2: uri\n"
            ":type arg2: string\n"
            ":returns: instance of Result\n"
            ":raises: ValueError if uri doesn't exist")
        .def("result", result_by_index, return_value_policy<return_by_value>(),
             "Get a Result instance by index.\n\n"
             ":param arg2: index\n"
             ":type arg2: int\n"
             ":returns: instance of Result\n"
             ":raises: ValueError if index is invalid")
        ;
}
