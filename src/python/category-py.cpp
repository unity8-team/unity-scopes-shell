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

static object getRenderer(shr::Category*)
{
    //TODO
    return object();
}

void export_category()
{
    class_<shr::Category>("Category", no_init)
        .def("id", &shr::Category::id)
        .def("title", &shr::Category::title)
        .def("icon", &shr::Category::icon)
        .def("header_link", &shr::Category::headerLink)
        .def("renderer", &getRenderer)
        // TODO results etc
        ;


}
