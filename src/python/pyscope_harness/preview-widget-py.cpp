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
    class_<shp::PreviewWidget>("PreviewWidget", no_init)
        .add_property("id", &shp::PreviewWidget::id)
        .add_property("type", &shp::PreviewWidget::type)
        .add_property("data", &shp::PreviewWidget::data)
        .def("trigger", &shp::PreviewWidget::trigger)
        ;
}
