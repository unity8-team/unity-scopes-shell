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

#include <scope-harness/results/result.h>
#include <scope-harness/matcher/result-matcher.h>

#include <unity/scopes/Variant.h>

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
namespace
{
static void check_variant(MatchResult& matchResult, const results::Result& result,
    const string& name, const sc::Variant& actualValue, const sc::Variant& expectedValue)
{
    if (!(actualValue == expectedValue))
    {
        auto actualValueString = actualValue.serialize_json();
        auto expectedValueString = expectedValue.serialize_json();
        // serialize_json includes a trailing carriage return
        actualValueString.pop_back();
        expectedValueString.pop_back();

        matchResult.failure(
                "Result with URI '" + result.uri() + "' has '" + name
                        + "' == '" + actualValueString + "' but expected '"
                        + expectedValueString + "'");
    }
}

static void check_variant(MatchResult& matchResult, const results::Result& result,
    const string& name, const sc::Variant& expectedValue)
{
    const auto& actualValue = result[name];
    check_variant(matchResult, result, name, actualValue, expectedValue);
}


static void check_string(MatchResult& matchResult, const results::Result& result,
             const string& name, const string& actualValue,
             const string& expectedValue)
{
    try
    {
        if (actualValue != expectedValue)
        {
            matchResult.failure(
                    "Result with URI '" + result.uri() + "' has '" + name
                            + "' == '" + actualValue + "' but expected '"
                            + expectedValue + "'");
        }
    }
    catch (exception& e)
    {
        matchResult.failure(
                "Result with URI '" + result.uri()
                        + "' does not contain expected property '" + name
                        + "'");
    }
}
}

struct ResultMatcher::Priv
{
    string m_uri;

    optional<string> m_dndUri;

    optional<string> m_title;

    optional<string> m_art;

    optional<string> m_subtitle;

    optional<string> m_emblem;

    optional<string> m_mascot;

    optional<sc::Variant> m_attributes;

    optional<sc::Variant> m_summary;

    optional<sc::Variant> m_background;

    vector<pair<string, sc::Variant>> m_properties;
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
    p->m_subtitle = other.p->m_subtitle;
    p->m_emblem = other.p->m_emblem;
    p->m_mascot = other.p->m_mascot;
    p->m_attributes = other.p->m_attributes;
    p->m_summary = other.p->m_summary;
    p->m_background = other.p->m_background;
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

ResultMatcher& ResultMatcher::subtitle(const string& subtitle)
{
    p->m_subtitle = subtitle;
    return *this;
}

ResultMatcher& ResultMatcher::emblem(const string& emblem)
{
    p->m_emblem = emblem;
    return *this;
}

ResultMatcher& ResultMatcher::mascot(const string& mascot)
{
    p->m_mascot = mascot;
    return *this;
}

ResultMatcher& ResultMatcher::attributes(const sc::Variant& attributes)
{
    p->m_attributes = attributes;
    return *this;
}

ResultMatcher& ResultMatcher::summary(const sc::Variant& summary)
{
    p->m_summary = summary;
    return *this;
}

ResultMatcher& ResultMatcher::background(const sc::Variant& background)
{
    p->m_background = background;
    return *this;
}

ResultMatcher& ResultMatcher::property(const string& name, const sc::Variant& value)
{
    p->m_properties.emplace_back(make_pair(name, value));
    return *this;
}

MatchResult ResultMatcher::match(const results::Result& result) const
{
    MatchResult matchResult;
    match(matchResult, result);
    return matchResult;
}

void ResultMatcher::match(MatchResult& matchResult, const results::Result& result) const
{
    check_string(matchResult, result, "uri", result.uri(), p->m_uri);
    if (p->m_dndUri)
    {
        check_string(matchResult, result, "dnd_uri", result.dnd_uri(),
                     p->m_dndUri.get());
    }
    if (p->m_title)
    {
        check_string(matchResult, result, "title", result.title(), p->m_title.get());
    }
    if (p->m_art)
    {
        check_string(matchResult, result, "art", result.art(), p->m_art.get());
    }
    if (p->m_subtitle)
    {
        check_string(matchResult, result, "subtitle", result.subtitle(), p->m_subtitle.get());
    }
    if (p->m_emblem)
    {
        check_string(matchResult, result, "emblem", result.emblem(), p->m_emblem.get());
    }
    if (p->m_mascot)
    {
        check_string(matchResult, result, "mascot", result.mascot(), p->m_mascot.get());
    }
    if (p->m_attributes)
    {
        check_variant(matchResult, result, "attributes", result.attributes(), p->m_attributes.get());
    }
    if (p->m_summary)
    {
        check_variant(matchResult, result, "summary", result.summary(), p->m_summary.get());
    }
    if (p->m_background)
    {
        check_variant(matchResult, result, "background", result.background(), p->m_background.get());
    }

    for (const auto& property : p->m_properties)
    {
        check_variant(matchResult, result, property.first, property.second);
    }
}

string ResultMatcher::getUri() const
{
    return p->m_uri;
}

}
}
}
