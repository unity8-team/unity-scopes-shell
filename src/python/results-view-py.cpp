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
#include <scope-harness/view/results-view.h>
#include <scope-harness/results/category.h>
#include <unity/shell/scopes/ScopeInterface.h>

using namespace boost::python;
namespace shv = unity::scopeharness::view;
namespace shr = unity::scopeharness::results;

// wrapper function to create python list
// from list returned by ResultsView::categories
static PyObject* getCategories(shv::ResultsView* view)
{
    list pylist;
    for (auto const cat: view->categories())
    {
        pylist.append(cat);
    }
    return incref(pylist.ptr());
}

void export_view()
{
    boost::python::register_ptr_to_python<std::shared_ptr<shv::ResultsView>>();

    shr::Category (shv::ResultsView::*category_by_row)(std::size_t) = &shv::ResultsView::category;
    shr::Category (shv::ResultsView::*category_by_id)(const std::string&) = &shv::ResultsView::category;

    enum_<unity::shell::scopes::ScopeInterface::Status>("SearchStatus")
        .value("OKAY", unity::shell::scopes::ScopeInterface::Status::Okay)
        .value("NO_INTERNET", unity::shell::scopes::ScopeInterface::Status::NoInternet)
        .value("NO_LOCATION_DATA", unity::shell::scopes::ScopeInterface::Status::NoLocationData)
        .value("UNKNOWN", unity::shell::scopes::ScopeInterface::Status::Unknown)
        ;

    class_<shv::ResultsView>("ResultsView", no_init)
        .add_property("scope_id", &shv::ResultsView::scopeId)
        .add_property("display_name", &shv::ResultsView::displayName)
        .add_property("icon_hint", &shv::ResultsView::iconHint)
        .add_property("description", &shv::ResultsView::description)
        .add_property("search_hint", &shv::ResultsView::searchHint)
        .add_property("shortcut", &shv::ResultsView::searchQuery)
        .add_property("customizations", &shv::ResultsView::customizations)
        .add_property("session_id", &shv::ResultsView::sessionId)
        .add_property("query_id", &shv::ResultsView::queryId)
        .add_property("categories", &getCategories)
        .add_property("search_query", &shv::ResultsView::searchQuery, &shv::ResultsView::setQuery)
        .def("set_active_scope", &shv::ResultsView::setActiveScope)
        .def("wait_for_results_change", &shv::ResultsView::waitForResultsChange)
        .def("override_category_json", &shv::ResultsView::overrideCategoryJson)
        .def("category", category_by_row)
        .def("category", category_by_id)
    ;
}
