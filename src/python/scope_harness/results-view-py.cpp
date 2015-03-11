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
static object getCategories(shv::ResultsView* view)
{
    list pylist;
    for (auto const cat: view->categories())
    {
        pylist.append(cat);
    }
    return pylist;
}

void export_results_view()
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

    class_<shv::ResultsView, bases<shv::AbstractView>, boost::noncopyable>("ResultsView",
                                                       "This is the main class for driving search and inspecting search results. "
                                                       "Set search_query property to invoke search, then inspect categories property "
                                                       "to access returned categories and their results. Use browse_department method to "
                                                       "change active department.",
                                                       no_init)
        .add_property("scope_id", &shv::ResultsView::scopeId)
        .add_property("display_name", &shv::ResultsView::displayName)
        .add_property("icon_hint", &shv::ResultsView::iconHint)
        .add_property("description", &shv::ResultsView::description)
        .add_property("search_hint", &shv::ResultsView::searchHint)
        .add_property("shortcut", &shv::ResultsView::shortcut)
        .add_property("customizations", &shv::ResultsView::customizations)
        .add_property("session_id", &shv::ResultsView::sessionId)
        .add_property("query_id", &shv::ResultsView::queryId)
        .add_property("categories", &getCategories)
        .add_property("search_query", &shv::ResultsView::query, &shv::ResultsView::setQuery)
        .add_property("active_scope", &shv::ResultsView::activeScope, &shv::ResultsView::setActiveScope)
        .add_property("department_id", &shv::ResultsView::departmentId)
        .add_property("alt_department_id", &shv::ResultsView::altDepartmentId)
        .add_property("has_departments", &shv::ResultsView::hasDepartments)
        .add_property("has_alt_departments", &shv::ResultsView::hasAltDepartments)
        .def("browse_department", &shv::ResultsView::browseDepartment,
             "Go to a specific department by id. Returns Department instance.", return_value_policy<return_by_value>())
        .def("browse_alt_department", &shv::ResultsView::browseAltDepartment,
             "Go to a specific alternate (e.g. the top-right selection filter if provided by the scope)"
             " department by id. Returns Department instance.",
             return_value_policy<return_by_value>()
            )
        .def("category", category_by_row, "Get Category instance by row index")
        .def("category", category_by_id, "Get Category instance by id")
    ;
}
