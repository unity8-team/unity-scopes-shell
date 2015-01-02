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
#include <scope-harness/results/category.h>
#include <scope-harness/matcher/match-result.h>
#include <scope-harness/matcher/category-matcher.h>
#include <scope-harness/matcher/result-matcher.h>
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

static PyObject* getFailuresWrapper(shm::MatchResult* matchRes)
{
    list pylist;
    for (auto const flr: matchRes->failures())
    {
        pylist.append(flr);
    }
    return incref(pylist.ptr());
}

static shm::MatchResult getMatchResultByResultList(shm::CategoryListMatcher* catListMatcher, const object& obj)
{
    // convert python list to category list (deque type)
    stl_input_iterator<shr::Category> begin(obj), end;
    std::deque<shr::Category> cats(begin, end);
    return catListMatcher->match(cats);
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

    class_<shm::MatchResult>("MatchResult", init<>())
        .add_property("success", &shm::MatchResult::success)
        .add_property("failures", &getFailuresWrapper)
        .add_property("concat_failures", &shm::MatchResult::concat_failures)
        .def("failure", &shm::MatchResult::failure)
        ;

    {
        shm::MatchResult (shm::ResultMatcher::*matchresult_by_result)(const shr::Result&) const = &shm::ResultMatcher::match;
        void (shm::ResultMatcher::*match_by_match_result_and_result)(shm::MatchResult&, const shr::Result&) const = &shm::ResultMatcher::match;

        class_<shm::ResultMatcher, boost::noncopyable>("ResultMatcher", init<const std::string&>())
            .add_property("uri", &shm::ResultMatcher::getUri)
            .def("dnd_uri", &shm::ResultMatcher::dndUri, return_value_policy<reference_existing_object>())
            .def("title", &shm::ResultMatcher::title, return_value_policy<reference_existing_object>())
            .def("art", &shm::ResultMatcher::art, return_value_policy<reference_existing_object>())
            .def("subtitle", &shm::ResultMatcher::subtitle, return_value_policy<reference_existing_object>())
            .def("emblem", &shm::ResultMatcher::emblem, return_value_policy<reference_existing_object>())
            .def("mascot", &shm::ResultMatcher::mascot, return_value_policy<reference_existing_object>())
            .def("attributes", &shm::ResultMatcher::attributes, return_value_policy<reference_existing_object>())
            .def("summary", &shm::ResultMatcher::summary, return_value_policy<reference_existing_object>())
            .def("background", &shm::ResultMatcher::background, return_value_policy<reference_existing_object>())
            .def("property", &shm::ResultMatcher::property, return_value_policy<reference_existing_object>())
            .def("match", matchresult_by_result, return_value_policy<return_by_value>())
            .def("match", match_by_match_result_and_result)
            ;
    }

    {
        shm::CategoryMatcher& (shm::CategoryMatcher::*by_result_matcher)(const shm::ResultMatcher&) = &shm::CategoryMatcher::result;
        shm::MatchResult (shm::CategoryMatcher::*match_result_by_category)(const shr::Category&) const = &shm::CategoryMatcher::match;
        void (shm::CategoryMatcher::*match_by_match_result_and_category)(shm::MatchResult&, const shr::Category&) const = &shm::CategoryMatcher::match;

        class_<shm::CategoryMatcher, boost::noncopyable>("CategoryMatcher", init<const std::string&>())
            .add_property("id", &getIdWrapper, &setIdWrapper)
            .def("mode", &shm::CategoryMatcher::mode, return_value_policy<reference_existing_object>())
            .def("title", &shm::CategoryMatcher::title, return_value_policy<reference_existing_object>())
            .def("icon", &shm::CategoryMatcher::icon, return_value_policy<reference_existing_object>())
            .def("header_link", &shm::CategoryMatcher::headerLink, return_value_policy<reference_existing_object>())
            .def("has_at_least", &shm::CategoryMatcher::hasAtLeast, return_value_policy<reference_existing_object>())
            .def("renderer", &shm::CategoryMatcher::renderer, return_value_policy<reference_existing_object>())
            .def("components", &shm::CategoryMatcher::components, return_value_policy<reference_existing_object>())
            .def("result", by_result_matcher, return_value_policy<reference_existing_object>())
            .def("match", match_result_by_category, return_value_policy<return_by_value>())
            .def("match", match_by_match_result_and_category)
            ;
    }

    {
        shm::CategoryListMatcher& (shm::CategoryListMatcher::*category_match)(const shm::CategoryMatcher&) = &shm::CategoryListMatcher::category;

        class_<shm::CategoryListMatcher, boost::noncopyable>("CategoryListMatcher", init<>())
            .def("mode", &shm::CategoryListMatcher::mode, return_value_policy<reference_existing_object>())
            .def("category", category_match, return_value_policy<reference_existing_object>())
            .def("has_at_least", &shm::CategoryListMatcher::hasAtLeast, return_value_policy<reference_existing_object>())
            .def("has_exactly", &shm::CategoryListMatcher::hasExactly, return_value_policy<reference_existing_object>())
            .def("match", getMatchResultByResultList)
            ;
    }
}
