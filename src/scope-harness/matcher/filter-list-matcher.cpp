/*
 * Copyright (C) 2016 Canonical, Ltd.
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

#include <scope-harness/matcher/filter-list-matcher.h>
#include <scope-harness/matcher/filter-matcher.h>

#include <boost/optional.hpp>

using namespace std;
using namespace boost;

namespace unity
{
namespace scopeharness
{
namespace matcher
{

struct FilterListMatcher::_Priv
{
    Mode m_mode = Mode::all;

    vector<FilterMatcher> m_filters;

    optional<size_t> m_hasAtLeast;

    optional<size_t> m_hasExactly;
};

FilterListMatcher::FilterListMatcher()
: p(new _Priv)
{
}

FilterListMatcher& FilterListMatcher::mode(FilterListMatcher::Mode mode)
{
    return *this;
}

FilterListMatcher& FilterListMatcher::filter(const FilterMatcher& filterMatcher)
{
    return *this;
}

FilterListMatcher& FilterListMatcher::filter(FilterMatcher&& filterMatcher)
{
    return *this;
}

FilterListMatcher& FilterListMatcher::hasAtLeast(std::size_t minimum)
{
    p->m_hasAtLeast = minimum;
    return *this;
}

FilterListMatcher& FilterListMatcher::hasExactly(std::size_t amount)
{
    p->m_hasExactly = amount;
    return *this;
}

MatchResult FilterListMatcher::match(const results::Filter::List& filtersList) const
{
    MatchResult matchResult;

    if (p->m_hasAtLeast && filtersList.size() < p->m_hasAtLeast.get())
    {
        matchResult.failure(
                "Expected at least " + to_string(p->m_hasAtLeast.get())
                        + " filters");
    }

    if (p->m_hasExactly && filtersList.size() != p->m_hasExactly.get())
    {
        matchResult.failure(
                "Expected exactly " + to_string(p->m_hasExactly.get())
                        + " filters");
    }

    return matchResult;
}

}
}
}
