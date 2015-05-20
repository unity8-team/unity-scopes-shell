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
#include <unity/scopes/Variant.h>
#include <scope-harness/matcher/match-result.h>
#include <scope-harness/matcher/settings-matcher.h>
#include <scope-harness/matcher/settings-option-matcher.h>

using namespace boost::python;
namespace shm = unity::scopeharness::matcher;

static shm::SettingsOptionMatcher& display_values_wrapper(shm::SettingsOptionMatcher* settingsOptionMatcher, const object& obj)
{
    // convert python list to variant array
    stl_input_iterator<unity::scopes::Variant> begin(obj), end;
    unity::scopes::VariantArray values(begin, end);
    return settingsOptionMatcher->displayValues(values);
}

void export_settings_matchers()
{
    enum_<shm::SettingsMatcher::Mode>("SettingsMatcherMode")
        .value("ALL", shm::SettingsMatcher::Mode::all)
        .value("BY_ID", shm::SettingsMatcher::Mode::by_id)
        .value("STARTS_WITH", shm::SettingsMatcher::Mode::starts_with)
        ;

    class_<shm::SettingsMatcher>("SettingsMatcher",
                                 "",
                                 init<>())
        .def("mode", &shm::SettingsMatcher::mode, return_internal_reference<1>())
        .def("option", &shm::SettingsMatcher::option, return_internal_reference<1>())
        .def("has_at_least", &shm::SettingsMatcher::hasAtLeast, return_internal_reference<1>())
        .def("has_exactly", &shm::SettingsMatcher::hasExactly, return_internal_reference<1>())
        .def("match", &shm::SettingsMatcher::match, return_value_policy<return_by_value>())
        ;

    class_<shm::SettingsOptionMatcher>("SettingsOptionMatcher",
                                       "",
                                       init<const std::string&>())
        .def("display_name", &shm::SettingsOptionMatcher::displayName, return_internal_reference<1>())
        .def("option_type", &shm::SettingsOptionMatcher::optionType, return_internal_reference<1>())
        .def("default_value", &shm::SettingsOptionMatcher::defaultValue, return_internal_reference<1>())
        .def("value", &shm::SettingsOptionMatcher::value, return_internal_reference<1>())
        .def("display_values", display_values_wrapper, return_internal_reference<1>())
        ;
}
