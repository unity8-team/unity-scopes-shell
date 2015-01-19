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
#include <boost/python/stl_iterator.hpp>
#include <scope-harness/matcher/preview-matcher.h>
#include <scope-harness/matcher/preview-widget-matcher.h>
#include <scope-harness/matcher/preview-column-matcher.h>

using namespace boost::python;
namespace shm = unity::scopeharness::matcher;
namespace shp = unity::scopeharness::preview;

static shm::MatchResult getMatchResultByPreviewWidgetsLists(shm::PreviewColumnMatcher* colMatcher, const object& obj)
{
    // convert python list to vector of widget lists
    stl_input_iterator<shp::PreviewWidgetList> begin(obj), end;
    std::vector<shp::PreviewWidgetList> wlists(begin, end);
    return colMatcher->match(wlists);
}

static void matchByMatchResultAndPreviewWidgetsLists(shm::PreviewColumnMatcher* colMatcher, shm::MatchResult& mr, const object& obj)
{
    // convert python list to vector of widget lists
    stl_input_iterator<shp::PreviewWidgetList> begin(obj), end;
    std::vector<shp::PreviewWidgetList> wlists(begin, end);
    return colMatcher->match(mr, wlists);
}

void export_preview_matchers()
{
    {
        shm::PreviewWidgetMatcher& (shm::PreviewMatcher::*data_by_variant)(const unity::scopes::Variant& data) = &shm::PreviewWidgetMatcher::data;
        shm::MatchResult (shm::PreviewWidgetMatcher::*matchresult_by_widget)(const shp::PreviewWidget&) const = &shm::PreviewWidgetMatcher::match;
        void (shm::PreviewMatcher::*match_by_matchresult_and_widget)(shm::MatchResult&, const shp::PreviewWidget&) const = &shm::PreviewWidgetMatcher::match;

        class_<shm::PreviewWidgetMatcher>("PreviewWidgetMatcher", init<const std::string&>())
            .def("type", &shm::PreviewWidgetMatcher::type, return_internal_reference<1>())
            .def("data", data_by_variant, return_internal_reference<1>())
            .def("match", matchresult_by_widget, return_value_policy<return_by_value>())
            .def("match", match_by_matchresult_and_widget)
            ;
    }

    {
        shm::PreviewMatcher& (shm::PreviewMatcher::*widget_by_widgetmatcher)(const shm::PreviewWidgetMatcher&) = &shm::PreviewMatcher::widget;
        shm::MatchResult (shm::PreviewMatcher::*match_by_matchresult_and_widgets)(const shp::PreviewWidgetList&) const = &shm::PreviewMatcher::match;
        void (shm::PreviewMatcher::*matchresult_by_widgets)(shm::MatchResult&, const shp::PreviewWidgetList&) const = &shm::PreviewMatcher::match;

        class_<shm::PreviewMatcher>("PreviewMatcher", init<>())
            .def("widget", widget_by_widgetmatcher, return_internal_reference<1>())
            .def("match", match_by_matchresult_and_widgets)
            .def("match", matchresult_by_widgets, return_value_policy<return_by_value>())
            ;
    }

    {
        shm::PreviewColumnMatcher& (shm::PreviewColumnMatcher::*column_by_previewmatcher)(const shm::PreviewMatcher&) = &shm::PreviewColumnMatcher::column;

        class_<shm::PreviewColumnMatcher>("PreviewColumnMatcher", init<>())
            .def("column", column_by_previewmatcher, return_internal_reference<1>())
            .def("match", getMatchResultByPreviewWidgetsLists, return_value_policy<return_by_value>())
            .def("match", matchByMatchResultAndPreviewWidgetsLists)
            ;
    }
}
