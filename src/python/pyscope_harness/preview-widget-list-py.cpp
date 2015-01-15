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
#include <unity/UnityExceptions.h>
#include <stdexcept>
#include <iostream>

using namespace boost::python;
namespace shp = unity::scopeharness::preview;

static shp::PreviewWidget previewWidgetListGetWrapper(shp::PreviewWidgetList* wlist, const object& index)
{
    if (PyLong_Check(index.ptr()))
    {
        std::cerr << "getitem: int\n";
        long intidx = extract<long>(index);
        if (intidx < 0)
        {
            intidx = static_cast<long>(wlist->size()) + intidx;
        }
        if (intidx < 0 || intidx >= static_cast<long>(wlist->size()))
        {
            throw std::out_of_range("PreviewWidgetList index out of range");
        }
        else
        {
            return wlist->at(intidx);
        }
    }
    else if (PyUnicode_Check(index.ptr()))
    {
        std::cerr << "getitem: string\n";
        std::string stridx = extract<std::string>(index);
        try
        {
            return wlist->at(stridx);
        }
        catch (const std::domain_error&)
        {
            throw std::out_of_range("PreviewWidgetList widget key '" + stridx + "' doesn't exist");
        }
    }
    else if (PySlice_Check(index.ptr()))
    {
        std::cerr << "getitem: slice\n";
        // TODO
        return wlist->at(0);
    }

    throw unity::InvalidArgumentException("PreviewWidgetList unsupported index type");
}

static PyObject* previewWidgetListMissingKey(shp::PreviewWidgetList* wlist, PyObject *index)
{
    return nullptr;
}

void export_preview_widget_list()
{
    class_<shp::PreviewWidgetList>("PreviewWidgetList", no_init)
        .def("__len__", &shp::PreviewWidgetList::size)
        .def("__getitem__", previewWidgetListGetWrapper)
        .def("__missing__", previewWidgetListMissingKey)
        ;
}
