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
#include <unordered_map>
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
    Mode m_mode = Mode::all;
    vector<SettingsOptionMatcher> m_options;
    optional<size_t> m_hasAtLeast;
    optional<size_t> m_hasExactly;

    void all(MatchResult& matchResult, const view::SettingsView& settings)
    {
        auto opts = settings.options();
        if (opts.size() != m_options.size())
        {
            matchResult.failure(
                    "Settings options list contained " + to_string(opts.size())
                            + " expected " + to_string(m_options.size()));
            return;
        }

        for (size_t row = 0; row < m_options.size(); ++row)
        {
            const auto& expected = m_options[row];
            const auto& actual = opts[row];
            expected.match(matchResult, actual);
        }
    }

    void byId(MatchResult& matchResult, const view::SettingsView& settings)
    {
        unordered_map<string, view::SettingsView::Option> optionsById;
        for (const auto& opt: settings.options())
        {
            optionsById.insert({opt.id, opt});
        }

        for (const auto& expected: m_options)
        {
            auto it = optionsById.find(expected.getId());
            if (it == optionsById.end())
            {
                matchResult.failure(
                        "Settings option with ID " + expected.getId()
                                + " could not be found");
            }
            else
            {
                expected.match(matchResult, it->second);
            }
        }
    }

    void startsWith(MatchResult& matchResult, const view::SettingsView& settings)
    {
        auto opts = settings.options();
        if (opts.size() < m_options.size())
        {
            matchResult.failure(
                    "Settings options list contained " + to_string(opts.size())
                            + " expected at least " + to_string(m_options.size()));
            return;
        }

        for (size_t row = 0; row < m_options.size(); ++row)
        {
            const auto& expected = m_options[row];
            const auto& actual = opts[row];
            expected.match(matchResult, actual);
        }

    }
};

SettingsMatcher::SettingsMatcher() :
    p(new _Priv)
{
}

SettingsMatcher& SettingsMatcher::mode(Mode mode)
{
    p->m_mode= mode;
    return *this;
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

MatchResult SettingsMatcher::match(const view::SettingsView::SPtr& settings) const
{
    MatchResult matchResult;

    if (p->m_hasAtLeast && settings->count() < p->m_hasAtLeast)
    {
        matchResult.failure(
                "Expected at least " + to_string(p->m_hasAtLeast.get()) + " options");
    }

    if (p->m_hasExactly && settings->count() != p->m_hasExactly)
    {
        matchResult.failure(
                "Expected exactly " + to_string(p->m_hasAtLeast.get()) + " options");
    }

    if (!p->m_options.empty())
    {
        switch (p->m_mode)
        {
            case Mode::all:
                p->all(matchResult, *settings);
                break;
            case Mode::by_id:
                p->byId(matchResult, *settings);
                break;
            case Mode::starts_with:
                p->startsWith(matchResult, *settings);
                break;
        }

    }

    return matchResult;
}


}
}
}
