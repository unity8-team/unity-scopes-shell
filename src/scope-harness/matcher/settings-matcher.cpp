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

#include <scope-harness/matcher/settings-matcher.h>
#include <scope-harness/matcher/settings-option-matcher.h>
#include <boost/optional.hpp>

using namespace std;
using namespace boost;

namespace unity
{
namespace scopeharness
{
namespace matcher
{

struct SettingsMatcher::_Priv
{
    vector<SettingsOptionMatcher> m_options;
    optional<size_t> m_hasAtLeast;
    optional<size_t> m_hasExactly;
};

SettingsMatcher::SettingsMatcher() :
    p(new _Priv)
{
}

SettingsMatcher& SettingsMatcher::hasAtLeast(size_t minimum)
{
    p->m_hasAtLeast = minimum;
    return *this;
}

SettingsMatcher& SettingsMatcher::hasExactly(size_t amount)
{
    p->m_hasExactly = amount;
    return *this;
}

SettingsMatcher& SettingsMatcher::option(const SettingsOptionMatcher& optionMatcher)
{
    p->m_options.emplace_back(optionMatcher);
    return *this;
}

MatchResult SettingsMatcher::match(const view::SettingsView& settings)
{
    MatchResult matchResult;

    if (p->m_hasAtLeast && settings.count() < p->m_hasAtLeast)
    {
        matchResult.failure(
                "Expected at least " + to_string(p->m_hasAtLeast.get()) + " options");
    }

    if (p->m_hasExactly && settings.count() != p->m_hasExactly)
    {
        matchResult.failure(
                "Expected exactly " + to_string(p->m_hasAtLeast.get()) + " options");
    }

    if (!p->m_options.empty())
    {
        //TODO
    }

    return matchResult;
}


}
}
}
