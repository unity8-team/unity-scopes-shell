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
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <scope-harness/result-matcher.h>

#include <boost/optional.hpp>

using namespace std;
using namespace boost;

namespace sc = unity::scopes;

namespace unity
{
namespace scopeharness
{

struct ResultMatcher::Priv
{
    string m_uri;

    optional<string> m_dndUri;

    optional<string> m_title;

    optional<string> m_art;

    deque<pair<string, sc::Variant>> m_properties;
};

ResultMatcher::ResultMatcher(const string& uri) :
        p(new Priv)
{
    p->m_uri = uri;
}

ResultMatcher::ResultMatcher(const ResultMatcher& other) :
    p(new Priv)
{
    *this = other;
}

ResultMatcher& ResultMatcher::operator=(const ResultMatcher& other)
{
    p->m_uri = other.p->m_uri;
    p->m_dndUri = other.p->m_dndUri;
    p->m_title = other.p->m_title;
    p->m_art = other.p->m_art;
    p->m_properties = other.p->m_properties;
    return *this;
}

ResultMatcher& ResultMatcher::operator=(ResultMatcher&& other)
{
    p = move(other.p);
    return *this;
}

ResultMatcher& ResultMatcher::dndUri(const string& dndUri)
{
    p->m_dndUri = dndUri;
    return *this;
}

ResultMatcher& ResultMatcher::title(const string& title)
{
    p->m_title = title;
    return *this;
}

ResultMatcher& ResultMatcher::art(const string& art)
{
    p->m_art = art;
    return *this;
}

ResultMatcher& ResultMatcher::property(const string& name, const sc::Variant& value)
{
    p->m_properties.emplace_back(make_pair(name, value));
    return *this;
}

MatchResult ResultMatcher::match(const unity::scopes::Result::SCPtr& result) const
{
    MatchResult matchResult;
    match(matchResult, result);
    return matchResult;
}

void ResultMatcher::match(MatchResult& matchResult, const unity::scopes::Result::SCPtr& result) const
{
    if (result->uri() != p->m_uri)
    {
        matchResult.failure(
                "Result has URI '" + result->uri() + "' but expected '"
                        + p->m_uri + "'");
    }

    if (p->m_dndUri && result->dnd_uri() != p->m_dndUri)
    {
        matchResult.failure(
                "Result with URI '" + result->uri() + "' has DND URI '"
                        + result->dnd_uri() + "' but expected '" + p->m_dndUri.get()
                        + "'");
    }

    if (p->m_title && result->title() != p->m_title)
    {
        matchResult.failure(
                "Result with URI '" + result->uri() + "' has title '"
                        + result->title() + "' but expected '" + p->m_title.get()
                        + "'");
    }

    if (p->m_art && result->art() != p->m_art)
    {
        matchResult.failure(
                "Result with URI '" + result->uri() + "' has art '"
                        + result->art() + "' but expected '" + p->m_art.get() + "'");
    }

    for (const auto& property : p->m_properties)
    {
        const auto& name = property.first;
        const auto& expectedValue = property.second;

        if (!result->contains(name))
        {
            matchResult.failure(
                    "Result with URI '" + result->uri()
                            + "' does not contain expected property '" + name
                            + "'");
            continue;
        }

        const auto& actualValue = (*result)[name];
        if (!(actualValue == expectedValue))
        {
            auto actualValueString = actualValue.serialize_json();
            auto expectedValueString = expectedValue.serialize_json();
            // serialize_json includes a trailing carriage return
            actualValueString.pop_back();
            expectedValueString.pop_back();

            matchResult.failure(
                    "Result with URI '" + result->uri() + "' has '" + name
                            + "' == '" + actualValueString + "' but expected '"
                            + expectedValueString + "'");
        }
    }
}

string ResultMatcher::getUri() const
{
    return p->m_uri;
}

}
}
