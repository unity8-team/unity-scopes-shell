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
#include <scope-harness/results/result.h>

using namespace boost::python;
namespace shr = unity::scopeharness::results;

void export_result()
{
    class_<shr::Result>("Result", "Represents a read-only view on a result returned by a scope",
                        no_init)
        .add_property("uri", &shr::Result::uri)
        .add_property("title", &shr::Result::title)
        .add_property("art", &shr::Result::art)
        .add_property("dnd_uri", &shr::Result::dnd_uri)
        .add_property("subtitle", &shr::Result::subtitle)
        .add_property("emblem", &shr::Result::emblem)
        .add_property("mascot", &shr::Result::mascot)
        .add_property("attributes", &shr::Result::attributes)
        .add_property("summary", &shr::Result::summary)
        .add_property("background", &shr::Result::background)
        .def("__getitem__", &shr::Result::value, return_internal_reference<1>())
        .def("activate", &shr::Result::activate, "Activates the result, as if user tapped it. "
             "Returns an instance of PreviewView (if result was previewed) or ResultsView "
             " (if result's uri was a canned scope query, resulting in a new search)")
        ;
}
