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
#include <boost/python/stl_iterator.hpp>
#include <unity/scopes/Variant.h>
#include <scope-harness/results/category.h>
#include <scope-harness/results/department.h>
#include <scope-harness/results/child-department.h>
#include <scope-harness/matcher/match-result.h>
#include <scope-harness/matcher/category-matcher.h>
#include <scope-harness/matcher/result-matcher.h>
#include <scope-harness/matcher/department-matcher.h>
#include <scope-harness/matcher/child-department-matcher.h>
#include <scope-harness/matcher/category-list-matcher.h>

using namespace boost::python;
namespace shm = unity::scopeharness::matcher;
namespace shr = unity::scopeharness::results;

static std::string getIdWrapper(shm::CategoryMatcher* m)
{
    return m->getId();
}

static void setIdWrapper(shm::CategoryMatcher *m, const std::string& id)
{
    m->getId() = id; //FIXME: is this the intended way of setting it?
}

static object getFailuresWrapper(shm::MatchResult* matchRes)
{
    list pylist;
    for (auto const flr: matchRes->failures())
    {
        pylist.append(flr);
    }
    return pylist;
}

static shm::MatchResult getMatchResultByResultList(shm::CategoryListMatcher* catListMatcher, const object& obj)
{
    // convert python list to category list (vector type)
    stl_input_iterator<shr::Category> begin(obj), end;
    std::vector<shr::Category> cats(begin, end);
    return catListMatcher->match(cats);
}

static shm::ResultMatcher& resultMatcherProperties(shm::ResultMatcher* rm, dict kwargs)
{
    // iterate over kwargs dict, every pair is a ResultMatcher property
    stl_input_iterator<object> begin(kwargs.items()), end;
    for (auto it = begin; it != end; ++it)
    {
        rm->property(extract<std::string>((*it)[0]), extract<unity::scopes::Variant>((*it)[1]));
    }
    return *rm;
}

void export_matchers()
{
    enum_<shm::CategoryListMatcher::Mode>("CategoryListMatcherMode")
        .value("ALL", shm::CategoryListMatcher::Mode::all)
        .value("BY_ID", shm::CategoryListMatcher::Mode::by_id)
        .value("STARTS_WITH", shm::CategoryListMatcher::Mode::starts_with)
        ;

    enum_<shm::CategoryMatcher::Mode>("CategoryMatcherMode")
        .value("ALL", shm::CategoryMatcher::Mode::all)
        .value("STARTS_WITH", shm::CategoryMatcher::Mode::starts_with)
        .value("BY_URI", shm::CategoryMatcher::Mode::by_uri)
        ;

    class_<shm::MatchResult>("MatchResult",
                             "Represents the result of matching and is the final object you want to check in your tests. "
                             "An instance of MatchResult can be obtained by calling one of the match() methods of "
                             "ResultMatcher, CategoryMatcher, CategoryListMatcher, DepartmentMatcher and ChildDepartmentMatcher. "
                             "When implementing tests on top of scope_harness.testing.ScopeHarnessTestCase class, use its assertMatchResult "
                             "helper method to assert on MatchResult instance.",
                             init<>())
        .add_property("success", &shm::MatchResult::success)
        .add_property("failures", &getFailuresWrapper)
        .add_property("concat_failures", &shm::MatchResult::concat_failures)
        .def("failure", &shm::MatchResult::failure)
        ;

    enum_<shm::DepartmentMatcher::Mode>("DepartmentMatcherMode")
        .value("ALL", shm::DepartmentMatcher::Mode::all)
        .value("BY_ID", shm::DepartmentMatcher::Mode::by_id)
        .value("STARTS_WITH", shm::DepartmentMatcher::Mode::starts_with)
        ;

    {
        shm::MatchResult (shm::ResultMatcher::*matchresult_by_result)(const shr::Result&) const = &shm::ResultMatcher::match;
        void (shm::ResultMatcher::*match_by_match_result_and_result)(shm::MatchResult&, const shr::Result&) const = &shm::ResultMatcher::match;

        class_<shm::ResultMatcher, boost::noncopyable>("ResultMatcher",
                                                       "Matcher object that holds constraints for matching search result.",
                                                       init<const std::string>())
            .add_property("uri", &shm::ResultMatcher::getUri)
            .def("dnd_uri", &shm::ResultMatcher::dndUri, return_internal_reference<1>())
            .def("title", &shm::ResultMatcher::title, return_internal_reference<1>())
            .def("art", &shm::ResultMatcher::art, return_internal_reference<1>())
            .def("subtitle", &shm::ResultMatcher::subtitle, return_internal_reference<1>())
            .def("emblem", &shm::ResultMatcher::emblem, return_internal_reference<1>())
            .def("mascot", &shm::ResultMatcher::mascot, return_internal_reference<1>())
            .def("attributes", &shm::ResultMatcher::attributes, return_internal_reference<1>())
            .def("summary", &shm::ResultMatcher::summary, return_internal_reference<1>())
            .def("background", &shm::ResultMatcher::background, return_internal_reference<1>())
            .def("property", &shm::ResultMatcher::property, return_internal_reference<1>())
            // wrapper for ResultMatcher::property() which takes kwargs to set multiple properties at a time
            .def("properties", &resultMatcherProperties, return_internal_reference<1>())
            .def("match", matchresult_by_result, return_value_policy<return_by_value>())
            .def("match", match_by_match_result_and_result)
            ;
    }

    {
        shm::CategoryMatcher& (shm::CategoryMatcher::*by_result_matcher)(const shm::ResultMatcher&) = &shm::CategoryMatcher::result;
        shm::MatchResult (shm::CategoryMatcher::*match_result_by_category)(const shr::Category&) const = &shm::CategoryMatcher::match;
        void (shm::CategoryMatcher::*match_by_match_result_and_category)(shm::MatchResult&, const shr::Category&) const = &shm::CategoryMatcher::match;

        class_<shm::CategoryMatcher, boost::noncopyable>("CategoryMatcher",
                                                         "Matcher object that holds constraints for matching search category.",
                                                         init<const std::string&>())
            .add_property("id", &getIdWrapper, &setIdWrapper)
            .def("mode", &shm::CategoryMatcher::mode, return_internal_reference<1>())
            .def("title", &shm::CategoryMatcher::title, return_internal_reference<1>())
            .def("icon", &shm::CategoryMatcher::icon, return_internal_reference<1>())
            .def("header_link", &shm::CategoryMatcher::headerLink, return_internal_reference<1>())
            .def("has_at_least", &shm::CategoryMatcher::hasAtLeast, return_internal_reference<1>())
            .def("renderer", &shm::CategoryMatcher::renderer, return_internal_reference<1>())
            .def("components", &shm::CategoryMatcher::components, return_internal_reference<1>())
            .def("result", by_result_matcher, return_internal_reference<1>())
            .def("match", match_result_by_category, return_value_policy<return_by_value>())
            .def("match", match_by_match_result_and_category)
            ;
    }

    {
        shm::CategoryListMatcher& (shm::CategoryListMatcher::*category_match)(const shm::CategoryMatcher&) = &shm::CategoryListMatcher::category;

        class_<shm::CategoryListMatcher, boost::noncopyable>("CategoryListMatcher",
                                                             "Matcher object that holds constraints for matching search categories.",
                                                             init<>())
            .def("mode", &shm::CategoryListMatcher::mode, return_internal_reference<1>())
            .def("category", category_match, return_internal_reference<1>())
            .def("has_at_least", &shm::CategoryListMatcher::hasAtLeast, return_internal_reference<1>())
            .def("has_exactly", &shm::CategoryListMatcher::hasExactly, return_internal_reference<1>())
            .def("match", getMatchResultByResultList, return_value_policy<return_by_value>())
            ;
    }

    {
        shm::MatchResult (shm::DepartmentMatcher::*matchresult_by_department)(const shr::Department&) const = &shm::DepartmentMatcher::match;
        void (shm::ResultMatcher::*match_by_match_result_and_department)(shm::MatchResult&, const shr::Department&) const = &shm::DepartmentMatcher::match;
        shm::DepartmentMatcher& (shm::DepartmentMatcher::*by_child_department_matcher)(const shm::ChildDepartmentMatcher&) = &shm::DepartmentMatcher::child;

        class_<shm::DepartmentMatcher, boost::noncopyable>("DepartmentMatcher",
                                                           "Matcher object that holds constraints for matching departments.",
                                                           init<>())
            .def("mode", &shm::DepartmentMatcher::mode, return_internal_reference<1>())
            .def("has_exactly", &shm::DepartmentMatcher::hasExactly, return_internal_reference<1>())
            .def("has_at_least", &shm::DepartmentMatcher::hasAtLeast, return_internal_reference<1>())
            .def("id", &shm::DepartmentMatcher::id, return_internal_reference<1>())
            .def("label", &shm::DepartmentMatcher::label, return_internal_reference<1>())
            .def("all_label", &shm::DepartmentMatcher::allLabel, return_internal_reference<1>())
            .def("parent_id", &shm::DepartmentMatcher::parentId, return_internal_reference<1>())
            .def("parent_label", &shm::DepartmentMatcher::parentLabel, return_internal_reference<1>())
            .def("is_root", &shm::DepartmentMatcher::isRoot, return_internal_reference<1>())
            .def("is_hidden", &shm::DepartmentMatcher::isHidden, return_internal_reference<1>())
            .def("child", by_child_department_matcher, return_internal_reference<1>())
            .def("match", matchresult_by_department, return_value_policy<return_by_value>())
            .def("match", match_by_match_result_and_department)
            ;
    }

    {
        shm::MatchResult (shm::ChildDepartmentMatcher::*matchresult_by_child_department)(const shr::ChildDepartment&) const = &shm::ChildDepartmentMatcher::match;
        void (shm::ChildDepartmentMatcher::*match_by_match_result_and_child_department)(shm::MatchResult&, const shr::ChildDepartment&) const = &shm::ChildDepartmentMatcher::match;

        class_<shm::ChildDepartmentMatcher>("ChildDepartmentMatcher",
                                            "Matcher object that holds constraints for matching child departments of a department.",
                                            init<const std::string&>())
            .add_property("id", &shm::ChildDepartmentMatcher::getId)
            .def("label", &shm::ChildDepartmentMatcher::label, return_internal_reference<1>())
            .def("has_children", &shm::ChildDepartmentMatcher::hasChildren, return_internal_reference<1>())
            .def("is_active", &shm::ChildDepartmentMatcher::isActive, return_internal_reference<1>())
            .def("match", matchresult_by_child_department, return_value_policy<return_by_value>())
            .def("match", match_by_match_result_and_child_department)
            ;
    }
}
