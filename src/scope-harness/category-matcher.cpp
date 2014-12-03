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

#include <scope-harness/category-matcher.h>
#include <scope-harness/result-matcher.h>

#include <boost/optional.hpp>

#include <unordered_map>

using namespace std;
using namespace boost;

namespace sc = unity::scopes;

namespace unity
{
namespace scopeharness
{

struct CategoryMatcher::Priv
{
    string m_id;

    Mode m_mode = Mode::all;

    deque<ResultMatcher> m_results;

    optional<unsigned int> m_hasAtLeast;

    optional<string> m_title;

    optional<string> m_icon;

    optional<string> m_headerLink;

    void all(MatchResult& matchResult, const ResultList& resultList)
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

    void startsWith(MatchResult& matchResult, const ResultList&)
    {
        matchResult.failure("Starts with not implemented");
    }

    void uri(MatchResult& matchResult, const ResultList& resultList)
    {
        unordered_map<string, sc::Result::SCPtr> resultsByUri;
        for (const auto& result : resultList)
        {
            resultsByUri[result->uri()] = result;
        }

        for (const auto& expectedResult : m_results)
        {
            auto it = resultsByUri.find(expectedResult.getUri());
            if (it == resultsByUri.end())
            {
                matchResult.failure(
                        "Result with URI " + expectedResult.getUri()
                                + " could not be found");
            }
            else
            {
                expectedResult.match(matchResult, it->second);
            }
        }
    }
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

CategoryMatcher& CategoryMatcher::operator=(const CategoryMatcher& other)
{
    p->m_id = other.p->m_id;
    p->m_mode = other.p->m_mode;
    p->m_results = other.p->m_results;
    p->m_hasAtLeast = other.p->m_hasAtLeast;
    p->m_title = other.p->m_title;
    p->m_icon = other.p->m_icon;
    p->m_headerLink = other.p->m_headerLink;
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

CategoryMatcher& CategoryMatcher::hasAtLeast(unsigned int minimum)
{
    p->m_hasAtLeast = minimum;
    return *this;
}

CategoryMatcher& CategoryMatcher::result(const ResultMatcher& resultMatcher)
{
    p->m_results.emplace_back(resultMatcher);
    return *this;
}

void CategoryMatcher::match(MatchResult& matchResult, const CategoryResultListPair& categoryPair) const
{
    const auto& category = categoryPair.first;
    const auto& results = categoryPair.second;

    if (p->m_id != category->id())
    {
        matchResult.failure("Category ID " + category->id() + " != " + p->m_id);
    }

    if (p->m_hasAtLeast && results.size() < p->m_hasAtLeast.get())
    {
        matchResult.failure(
                "Category with ID " + category->id() + " contains only "
                        + to_string(results.size())
                        + " results. Expected at least "
                        + to_string(p->m_hasAtLeast.get()));
    }

    if (p->m_title && category->title() != p->m_title)
    {
        matchResult.failure(
                "Category with ID " + category->id() + ", title '"
                        + category->title() + "' != '" + p->m_title.get()
                        + "'");
    }

    if (p->m_icon && category->icon() != p->m_icon)
    {
        matchResult.failure(
                "Category with ID " + category->icon() + ", icon '"
                        + category->icon() + "' != '" + p->m_icon.get() + "'");
    }

    // TODO Support for header link. Where does it come from?

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
            case Mode::uri:
                p->uri(matchResult, results);
                break;
        }
    }
}

MatchResult CategoryMatcher::match(const CategoryResultListPair& category) const
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
