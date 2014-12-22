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
#include <scope-harness/scope-harness.h>

using namespace boost::python;

void export_scopeharness()
{
    namespace sh = unity::scopeharness;

    class_<sh::ScopeHarness>("ScopeHarness", no_init)
        .add_property("results_view", &sh::ScopeHarness::resultsView)
        .def("new_from_pre_existing_config", &sh::ScopeHarness::newFromPreExistingConfig).staticmethod("new_from_pre_existing_config")
        .def("new_from_scope_list", &sh::ScopeHarness::newFromScopeList).staticmethod("new_from_scope_list")
        .def("new_from_system", &sh::ScopeHarness::newFromSystem).staticmethod("new_from_system")
    ;
}
