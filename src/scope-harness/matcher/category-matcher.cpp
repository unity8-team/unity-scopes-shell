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

#include <scope-harness/matcher/category-matcher.h>
#include <scope-harness/matcher/result-matcher.h>
#include <scope-harness/results/category.h>
#include <scope-harness/results/result.h>

#include <boost/optional.hpp>
#include <boost/regex.hpp>

#include <unordered_map>

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
static void check_variant(MatchResult& matchResult, const results::Category& category,
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
                "Category with ID '" + category.id() + "' has '" + name
                        + "' == '" + actualValueString + "' but expected '"
                        + expectedValueString + "'");
    }
}

static void check_string(MatchResult& matchResult, const results::Category& category,
             const string& name, const string& actualValue,
             const string& expectedValue)
{
    try
    {
        if (actualValue != expectedValue)
        {
            matchResult.failure(
                    "Category with ID '" + category.id() + "' has '" + name
                            + "' == '" + actualValue + "' but expected '"
                            + expectedValue + "'");
        }
    }
    catch (std::exception& e)
    {
        matchResult.failure(
                "Category with ID '" + category.id()
                        + "' does not contain expected property '" + name
                        + "'");
    }
}
}

struct CategoryMatcher::Priv
{
    void all(MatchResult& matchResult, const results::Result::List& resultList)
    {
        if (resultList.size() != m_results.size())
        {
            matchResult.failure(
                    "Result list contained " + to_string(resultList.size())
                            + " expected " + to_string(m_results.size()));
            return;
        }

        for (size_t row = 0; row < m_results.size(); ++row)
        {
            const auto& expectedResult = m_results[row];
            const auto& actualResult = resultList[row];
            expectedResult.match(matchResult, actualResult);
        }
    }

    void startsWith(MatchResult& matchResult, const results::Result::List&resultList)
    {
        if (resultList.size() < m_results.size())
        {
            matchResult.failure(
                    "Result list contained " + to_string(resultList.size())
                            + " expected at least " + to_string(m_results.size()));
            return;
        }

        for (size_t row = 0; row < m_results.size(); ++row)
        {
            const auto& expectedResult = m_results[row];
            const auto& actualResult = resultList[row];
            expectedResult.match(matchResult, actualResult);
        }
    }

    void byUri(MatchResult& matchResult, const results::Result::List& resultList)
    {
        for (const auto& expectedResult : m_results)
        {
            regex e(expectedResult.getUri());
            bool matched = false;
            for (const auto& result : resultList)
            {
                if (regex_match(result.uri(), e)) {
                    matched = true;
                    expectedResult.match(matchResult, result);
                    break;
                }
            }

            if (!matched)
            {
                matchResult.failure(
                        "Result with URI " + expectedResult.getUri()
                                + " could not be found");
            }
        }
    }

    string m_id;

    Mode m_mode = Mode::all;

    vector<ResultMatcher> m_results;

    optional<size_t> m_hasAtLeast;

    optional<string> m_title;

    optional<string> m_icon;

    optional<string> m_headerLink;

    optional<sc::Variant> m_renderer;

    optional<sc::Variant> m_components;
};

CategoryMatcher::CategoryMatcher(const string& id) :
        p(new Priv)
{
    p->m_id = id;
}

CategoryMatcher::CategoryMatcher(const CategoryMatcher& other) :
        p(new Priv)
{
    *this = other;
}

CategoryMatcher::CategoryMatcher(CategoryMatcher&& other)
{
    *this = move(other);
}

CategoryMatcher& CategoryMatcher::operator=(const CategoryMatcher& other)
{
    p->m_id = other.p->m_id;
    p->m_mode = other.p->m_mode;
    p->m_results = other.p->m_results;
    p->m_hasAtLeast = other.p->m_hasAtLeast;
    p->m_title = other.p->m_title;
    p->m_icon = other.p->m_icon;
    p->m_headerLink = other.p->m_headerLink;
    p->m_renderer = other.p->m_renderer;
    p->m_components = other.p->m_components;
    return *this;
}

CategoryMatcher& CategoryMatcher::operator=(CategoryMatcher&& other)
{
    p = move(other.p);
    return *this;
}

CategoryMatcher& CategoryMatcher::mode(Mode mode)
{
    p->m_mode = mode;
    return *this;
}

CategoryMatcher& CategoryMatcher::title(const string& title)
{
    p->m_title = title;
    return *this;
}

CategoryMatcher& CategoryMatcher::icon(const string& icon)
{
    p->m_icon = icon;
    return *this;
}

CategoryMatcher& CategoryMatcher::headerLink(const string& headerLink)
{
    p->m_headerLink = headerLink;
    return *this;
}

CategoryMatcher& CategoryMatcher::hasAtLeast(size_t minimum)
{
    p->m_hasAtLeast = minimum;
    return *this;
}

CategoryMatcher& CategoryMatcher::renderer(const sc::Variant& renderer)
{
    p->m_renderer = renderer;
    return *this;
}

CategoryMatcher& CategoryMatcher::components(const sc::Variant& components)
{
    p->m_components = components;
    return *this;
}

CategoryMatcher& CategoryMatcher::result(const ResultMatcher& resultMatcher)
{
    p->m_results.emplace_back(resultMatcher);
    return *this;
}

CategoryMatcher& CategoryMatcher::result(ResultMatcher&& resultMatcher)
{
    p->m_results.emplace_back(move(resultMatcher));
    return *this;
}

void CategoryMatcher::match(MatchResult& matchResult, const results::Category& category) const
{
    auto results = category.results();

    if (p->m_id != category.id())
    {
        matchResult.failure("Category ID " + category.id() + " != " + p->m_id);
    }

    if (p->m_hasAtLeast && results.size() < p->m_hasAtLeast.get())
    {
        matchResult.failure(
                "Category with ID " + category.id() + " contains only "
                        + to_string(results.size())
                        + " results. Expected at least "
                        + to_string(p->m_hasAtLeast.get()));
    }

    if (p->m_title)
    {
        check_string(matchResult, category, "title", category.title(), p->m_title.get());
    }

    if (p->m_icon)
    {
        check_string(matchResult, category, "icon", category.icon(), p->m_icon.get());
    }

    if (p->m_headerLink)
    {
        check_string(matchResult, category, "header_link", category.headerLink(), p->m_headerLink.get());
    }

    if (p->m_renderer)
    {
        check_variant(matchResult, category, "renderer", category.renderer(), p->m_renderer.get());
    }

    if (p->m_components)
    {
        check_variant(matchResult, category, "components", category.components(), p->m_components.get());
    }

    if (!p->m_results.empty())
    {
        switch (p->m_mode)
        {
            case Mode::all:
                p->all(matchResult, results);
                break;
            case Mode::starts_with:
                p->startsWith(matchResult, results);
                break;
            case Mode::by_uri:
                p->byUri(matchResult, results);
                break;
        }
    }
}

MatchResult CategoryMatcher::match(const results::Category& category) const
{
    MatchResult matchResult;
    match(matchResult, category);
    return matchResult;
}

string& CategoryMatcher::getId() const
{
    return p->m_id;
}

}
}
}
