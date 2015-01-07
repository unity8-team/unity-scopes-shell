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
#include <boost/python/stl_iterator.hpp>
#include <scope-harness/registry/custom-registry.h>
#include <vector>
#include <memory>

using namespace boost::python;

namespace shr = unity::scopeharness::registry;

static std::shared_ptr<shr::CustomRegistry::Parameters> makeParameters(const object& obj)
{
    stl_input_iterator<std::string> begin(obj), end;
    std::deque<std::string> prm(begin, end);
    return std::make_shared<shr::CustomRegistry::Parameters>(prm);
}

static void enableScopes(shr::CustomRegistry::Parameters* p, bool system, bool click, bool
        oem, bool remote)
{
    if (system)
    {
        p->includeSystemScopes();
    }
    if (click)
    {
        p->includeClickScopes();
    }
    if (oem)
    {
        p->includeOemScopes();
    }
    if (remote)
    {
        p->includeRemoteScopes();
    }
}

void export_registry()
{
    class_<shr::CustomRegistry::Parameters>("Parameters", no_init)
        .def("__init__", make_constructor(&makeParameters))
        .def("include_system_scopes", &shr::CustomRegistry::Parameters::includeSystemScopes, return_internal_reference<1>())
        .def("include_click_scopes", &shr::CustomRegistry::Parameters::includeClickScopes, return_internal_reference<1>())
        .def("include_oem_scopes", &shr::CustomRegistry::Parameters::includeOemScopes, return_internal_reference<1>())
        .def("include_remote_scopes", &shr::CustomRegistry::Parameters::includeRemoteScopes, return_internal_reference<1>())

        // convienience python method that takes named arguments
        .def("enable_scopes", &enableScopes, (arg("system_scopes") = false, arg("click_scopes") = false, arg("remote_scopes") = false),
                "Enable particular types of scopes via named arguments")
        ;

    class_<shr::CustomRegistry>("CustomRegistry", init<const shr::CustomRegistry::Parameters&>())
        .def("start", &shr::CustomRegistry::start)
    ;
}
