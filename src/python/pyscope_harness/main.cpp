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

using namespace boost::python;

void export_exceptions();
void export_variant();
void export_registry();
void export_abstract_view();
void export_results_view();
void export_preview_widget();
void export_preview_widget_list();
void export_preview_view();
void export_category();
void export_result();
void export_scopeharness();
void export_matchers();

BOOST_PYTHON_MODULE(_pyscope_harness)
{
    // enable custom docstring, disable auto-generated docstring including c++ signatures
    docstring_options local_docstring_options(true, true, false);

    export_exceptions();
    export_variant();
    export_registry();
    export_scopeharness();
    export_preview_widget();
    export_preview_widget_list();
    export_abstract_view();
    export_preview_view();
    export_results_view();
    export_category();
    export_result();
    export_matchers();
}
