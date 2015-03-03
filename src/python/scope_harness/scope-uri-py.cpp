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
#include <scope-harness/matcher/scope-uri.h>

using namespace boost::python;
namespace shm = unity::scopeharness::matcher;

void export_scope_uri()
{
    class_<shm::ScopeUri>("ScopeUri", "Helper class for creating scope:// uris", init<const std::string&>())
        .add_property("uri", &shm::ScopeUri::toString)
        .def("department", &shm::ScopeUri::department, return_internal_reference<1>(), "Set department of this scope uri")
        .def("query", &shm::ScopeUri::query, return_internal_reference<1>(), "Set search query string of this scope uri")
        .def("to_string", &shm::ScopeUri::toString)
        ;
}
