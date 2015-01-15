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
#include <scope-harness/view/preview-view.h>

using namespace boost::python;
namespace shv = unity::scopeharness::view;

// wrapper function to create python list of lists from a vector of lists
// returned by PreviewView::widgets()
static PyObject* previewViewWidgetsWrapper(shv::PreviewView* view)
{
    list pylist;
    for (auto const widget: view->widgets())
    {
        pylist.append(widget);
    }
    return incref(pylist.ptr());
}

static PyObject* previewViewWidgetsInColumnWrapper(shv::PreviewView* view, std::size_t column)
{
    list pylist;
    return incref(pylist.ptr());
}

static PyObject* previewViewWidgetsInFirstColumnWrapper(shv::PreviewView* view)
{
    list pylist;
    return incref(pylist.ptr());
}

void export_preview_view()
{
    class_<shv::PreviewView, bases<shv::AbstractView>>("PreviewView", no_init)
        .add_property("column_count", &shv::PreviewView::columnCount, &shv::PreviewView::setColumnCount)
        .add_property("widgets", &previewViewWidgetsWrapper)
        .def("widgets_in_column", &previewViewWidgetsInColumnWrapper)
        .add_property("widgets_in_first_column", &previewViewWidgetsInFirstColumnWrapper)
        ;
}
