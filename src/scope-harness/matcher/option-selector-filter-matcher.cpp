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

#include <scope-harness/matcher/option-selector-filter-matcher.h>

using namespace std;

namespace unity
{
namespace scopeharness
{
namespace matcher
{

OptionSelectorFilterMatcher::OptionSelectorFilterMatcher(const std::string& id)
    : FilterMatcher(id)
{
}

OptionSelectorFilterMatcher::OptionSelectorFilterMatcher(const OptionSelectorFilterMatcher& other)
    : FilterMatcher(other)
{
}

OptionSelectorFilterMatcher::OptionSelectorFilterMatcher(OptionSelectorFilterMatcher&& other)
    : FilterMatcher(other)
{
}

OptionSelectorFilterMatcher& OptionSelectorFilterMatcher::operator=(const OptionSelectorFilterMatcher& other)
{

}

OptionSelectorFilterMatcher& OptionSelectorFilterMatcher::operator=(OptionSelectorFilterMatcher&& other)
{

}

OptionSelectorFilterMatcher& OptionSelectorFilterMatcher::title(const std::string& title)
{
    return *this;
}

OptionSelectorFilterMatcher& OptionSelectorFilterMatcher::label(const std::string& label)
{
    return *this;
}

OptionSelectorFilterMatcher& OptionSelectorFilterMatcher::hasAtLeast(std::size_t minimum)
{
    return *this;
}

OptionSelectorFilterMatcher& OptionSelectorFilterMatcher::hasExactly(std::size_t amount)
{
    return *this;
}

MatchResult OptionSelectorFilterMatcher::match(const results::Filter& filter) const
{

}

void OptionSelectorFilterMatcher::match(MatchResult& matchResult, const results::Filter& filter) const
{

}

}
}
}
