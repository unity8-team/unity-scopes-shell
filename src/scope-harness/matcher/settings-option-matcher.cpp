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

#include <scope-harness/matcher/settings-option-matcher.h>
#include <boost/optional.hpp>

using namespace std;
using namespace boost;

namespace sc = unity::scopes;

namespace unity
{
namespace scopeharness
{
namespace matcher
{

struct SettingsOptionMatcher::_Priv
{
    std::string m_optionId;
    optional<view::SettingsView::OptionType> m_optionType;
    optional<string> m_displayName;
    optional<sc::Variant> m_defaultValue;
    optional<sc::Variant> m_value;
};

SettingsOptionMatcher::SettingsOptionMatcher(const std::string& optionId)
    : p(new _Priv)
{
    p->m_optionId = optionId;
}

SettingsOptionMatcher::SettingsOptionMatcher(const SettingsOptionMatcher& other)
    : p(new _Priv)
{
    *this = other;
}

SettingsOptionMatcher& SettingsOptionMatcher::operator=(const SettingsOptionMatcher& other)
{
    p->m_optionId = other.p->m_optionId;
    p->m_optionType = other.p->m_optionType;
    p->m_displayName = other.p->m_displayName;
    p->m_defaultValue = other.p->m_defaultValue;
    p->m_value = other.p->m_value;
    return *this;
}

SettingsOptionMatcher& SettingsOptionMatcher::displayName(const std::string& name)
{
    p->m_displayName = name;
    return *this;
}

SettingsOptionMatcher& SettingsOptionMatcher::optionType(view::SettingsView::OptionType optionType)
{
    p->m_optionType = optionType;
    return *this;
}

SettingsOptionMatcher& SettingsOptionMatcher::defaultValue(const unity::scopes::Variant& value)
{
    p->m_defaultValue = value;
    return *this;
}

SettingsOptionMatcher& SettingsOptionMatcher::value(const unity::scopes::Variant& value)
{
    p->m_value = value;
    return *this;
}

void SettingsOptionMatcher::match(MatchResult& matchResult, const view::SettingsView::Option& option)
{
}

}
}
}
