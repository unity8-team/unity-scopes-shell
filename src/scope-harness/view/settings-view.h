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

#pragma once

#include <QtGlobal>
#include <scope-harness/view/abstract-view.h>
#include <unity/scopes/Variant.h>

namespace unity
{
namespace scopeharness
{
namespace view
{

class Q_DECL_EXPORT SettingsView final: public AbstractView
{
public:
    UNITY_DEFINES_PTRS(SettingsView);

    enum OptionType
    {
        String
    };

    struct Q_DECL_EXPORT Option
    {
        std::string id;
        std::string displayName;
        unity::scopes::Variant defaultValue;
        unity::scopes::Variant value;
    };

    SettingsView(const SettingsView&) = delete;
    SettingsView(SettingsView&&) = delete;
    SettingsView& operator=(const SettingsView&) = delete;
    SettingsView& operator=(SettingsView&&) = delete;

    std::size_t count() const;

    void set(const std::string& option_id, const unity::scopes::Variant &value);

protected:
    struct _Priv;
    std::shared_ptr<_Priv> p;
};
}
}
}

/*
   resultsView->settings()->set("foo", "Bar");

   QVERIFY_MATCHRESULT(
    SettingsMatcher().option(
        SettingOptionMatcher("foo")
            .displayName("Foo")
            .optionType(SettingsMatcher::Type::String)
            .defaultValue("def")
        ).match(resultsView->settings())
    );
*/
