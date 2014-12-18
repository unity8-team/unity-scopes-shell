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

void export_registry();
void export_view();
void export_scopeharness();

BOOST_PYTHON_MODULE(harnesspy)
{
    // enable custom docstring, disable auto-generated docstring including c++ signatures
    docstring_options local_docstring_options(true, true, false);

    export_registry();
    export_scopeharness();
    export_view();
}
