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

static string to_string(view::SettingsView::OptionType t)
{
    if (t == view::SettingsView::OptionType::String)
    {
        return "string";
    }

    throw std::logic_error("Unexpected settings option type");
}

static void check(MatchResult& matchResult, const view::SettingsView::Option& option,
             const string& name, const string& actualValue,
             const string& expectedValue)
{
    if (actualValue != expectedValue)
    {
        matchResult.failure(
                "Option with ID '" + option.id + "' has '" + name
                + "' == '" + actualValue + "' but expected '"
                + expectedValue + "'");
    }
}

static void check(MatchResult& matchResult, const view::SettingsView::Option& option,
             const string& name, const sc::Variant& actualValue,
             const sc::Variant& expectedValue)
{
    if (!(actualValue == expectedValue))
    {
        auto actualValueString = actualValue.serialize_json();
        auto expectedValueString = expectedValue.serialize_json();
        // serialize_json includes a trailing carriage return
        actualValueString.pop_back();
        expectedValueString.pop_back();

        matchResult.failure(
                "Option with ID '" + option.id + "' has '" + name
                        + "' == '" + actualValueString + "' but expected '"
                        + expectedValueString + "'");
    }
}

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

std::string& SettingsOptionMatcher::getId() const
{
    return p->m_optionId;
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

SettingsOptionMatcher& SettingsOptionMatcher::defaultValue(const sc::Variant& value)
{
    p->m_defaultValue = value;
    return *this;
}

SettingsOptionMatcher& SettingsOptionMatcher::value(const sc::Variant& value)
{
    p->m_value = value;
    return *this;
}

void SettingsOptionMatcher::match(MatchResult& matchResult, const view::SettingsView::Option& option) const
{
    if (p->m_optionId != option.id)
    {
        matchResult.failure("Option ID " + option.id + " != " + p->m_optionId);
    }

    if (p->m_optionType && p->m_optionType != option.optionType)
    {
        matchResult.failure("Option ID " + option.id + " is of type " + to_string(option.optionType) + ", expected " + to_string(p->m_optionType.get()));
    }

    if (p->m_displayName)
    {
        check(matchResult, option, "displayName", option.displayName, p->m_displayName.get());
    }

    if (p->m_defaultValue)
    {
        check(matchResult, option, "defaultValue", option.defaultValue, p->m_defaultValue.get());
    }

    if (p->m_value)
    {
        check(matchResult, option, "value", option.value, p->m_value.get());
    }
}

}
}
}
