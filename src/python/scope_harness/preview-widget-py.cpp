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
#include <scope-harness/preview/preview-widget.h>

using namespace boost::python;
namespace shp = unity::scopeharness::preview;

void export_preview_widget()
{
    class_<shp::PreviewWidget>("PreviewWidget",
                               "This class represents a single widget of a preview, such as an image or header. "
                               "The 'data' property is a regular python dictionary "
                               "that corresponds to Scopes API VariantMap and contains actual key-values that constitute the "
                               "widget. See the documentation of Unity Scopes API for more information about supported values.",
                               no_init)
        .add_property("id", &shp::PreviewWidget::id)
        .add_property("type", &shp::PreviewWidget::type)
        .add_property("data", &shp::PreviewWidget::data)
        .def("trigger", &shp::PreviewWidget::trigger,
                "Trigger preview action.\n\n"
                ":param arg2: action identifier\n"
                ":type arg2: string\n"
                ":param arg3: dictionary holding additional action data, typically the value of ``data`` property.\n"
                ":type arg3: dict\n"
                ":returns: instance of PreviewView or ResultsView.\n"
                ":raises: ValueError if action couldn't be executed"
                )
        ;
}
