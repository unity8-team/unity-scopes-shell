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
#include <scope-harness/view/settings-view.h>

using namespace boost::python;
namespace shv = unity::scopeharness::view;

// wrapper function to create python list from a vector of options
// returned by SettingsView::options().
static object settingsViewOptionsWrapper(shv::SettingsView* view)
{
    list pylist;
    for (auto const opt: view->options())
    {
        pylist.append(opt);
    }
    return pylist;
}

// wrapper function to create python list from a vector of variants
// held by Option::displayValues.
static object optionDisplayValuesWrapper(shv::SettingsView::Option* opt)
{
    list pylist;
    for (auto const v: opt->displayValues)
    {
        pylist.append(v);
    }
    return pylist;
}

void export_settings_view()
{
    boost::python::register_ptr_to_python<std::shared_ptr<shv::SettingsView>>();

    enum_<shv::SettingsView::OptionType>("SettingsOptionType")
        .value("STRING", shv::SettingsView::OptionType::String)
        .value("NUMBER", shv::SettingsView::OptionType::Number)
        .value("LIST", shv::SettingsView::OptionType::List)
        .value("BOOLEAN", shv::SettingsView::OptionType::Boolean)
        ;

    class_<shv::SettingsView::Option>("SettingsOption", "This is a class holding properies of an option", no_init)
        .add_property("id", &shv::SettingsView::Option::id)
        .add_property("display_name", &shv::SettingsView::Option::displayName)
        .add_property("value", &shv::SettingsView::Option::value)
        .add_property("default_value", &shv::SettingsView::Option::defaultValue)
        .add_property("display_values", optionDisplayValuesWrapper)
        .add_property("option_type", &shv::SettingsView::Option::optionType);

    class_<shv::SettingsView, bases<shv::AbstractView>, boost::noncopyable>("SettingsView",
                                                       "This is a view on a scope settings returned by settings() method of ResultsView.",
                                                       no_init)
        .add_property("count", &shv::SettingsView::count)
        .add_property("options", settingsViewOptionsWrapper)
        .def("set", &shv::SettingsView::set, "Set value of an option")
        .def("__len__", &shv::SettingsView::count)
        ;
}
