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
#include <scope-harness/preview/preview-widget-list.h>
#include <stdexcept>

using namespace boost::python;
namespace shp = unity::scopeharness::preview;

static shp::PreviewWidget previewWidgetListGetByInt(shp::PreviewWidgetList* wlist, long index)
{
    if (index < 0)
    {
        index = static_cast<long>(wlist->size()) + index;
    }
    if (index < 0 || index >= static_cast<long>(wlist->size()))
    {
        throw std::out_of_range("PreviewWidgetList index out of range");
    }
    return wlist->at(index);
}

static shp::PreviewWidget previewWidgetListGetByString(shp::PreviewWidgetList* wlist, const std::string& key)
{
    try
    {
        return wlist->at(key);
    }
    catch (const std::domain_error&)
    {
        throw std::out_of_range("PreviewWidgetList widget key '" + key + "' doesn't exist");
    }
}

void export_preview_widget_list()
{
    class_<shp::PreviewWidgetList>("PreviewWidgetList",
                                   "A simple container for preview widgets returned by a scope. "
                                   "The number of widgets can be determined with python's len() function "
                                   "and individual widgets can be accessed using [] operator. "
                                   "The [] indexing operator supports numeric index values as well as "
                                   "string keys for accessing widgets based on their identifiers. "
                                   "Slices are not supported.",
                                   no_init)
        .def("__len__", &shp::PreviewWidgetList::size)
        .def("__getitem__", previewWidgetListGetByInt)
        .def("__getitem__", previewWidgetListGetByString)
        // it would be nice to support slice, but that's not possible unless PreviewWidgetList
        // provides ctors and ideally iterators
        ;
}
