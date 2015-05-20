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
#include <memory>

#include <unity/scopes/Variant.h>

#include <scope-harness/view/settings-view.h>
#include <scope-harness/matcher/match-result.h>

namespace unity
{
namespace scopeharness
{
namespace matcher
{

class Q_DECL_EXPORT SettingsOptionMatcher final
{
public:
    SettingsOptionMatcher(const std::string& optionId);
    SettingsOptionMatcher(const SettingsOptionMatcher&);
    SettingsOptionMatcher(SettingsOptionMatcher&&) = default;
    SettingsOptionMatcher& operator=(const SettingsOptionMatcher&);
    SettingsOptionMatcher& operator=(SettingsOptionMatcher&&) = default;

    SettingsOptionMatcher& displayName(const std::string& name);
    SettingsOptionMatcher& optionType(view::SettingsView::OptionType optionType);
    SettingsOptionMatcher& defaultValue(const unity::scopes::Variant& value);
    SettingsOptionMatcher& value(const unity::scopes::Variant& value);
    SettingsOptionMatcher& displayValues(const unity::scopes::VariantArray& values);

    std::string getId() const;

    void match(MatchResult& matchResult, const view::SettingsView::Option& option) const;

protected:
    struct _Priv;
    std::shared_ptr<_Priv> p;
};

}
}
}
