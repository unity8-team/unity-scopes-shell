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

#pragma once

#include <scope-harness/matcher/match-result.h>
#include <scope-harness/matcher/filter-matcher.h>
#include <QtGlobal>

namespace unity
{
namespace scopes
{
class Variant;
}
namespace scopeharness
{

namespace matcher
{

class Q_DECL_EXPORT OptionSelectorFilterMatcher: public FilterMatcher
{
public:
    OptionSelectorFilterMatcher(const std::string& id);

    OptionSelectorFilterMatcher(const OptionSelectorFilterMatcher& other);

    OptionSelectorFilterMatcher(OptionSelectorFilterMatcher&& other);

    OptionSelectorFilterMatcher& operator=(const OptionSelectorFilterMatcher& other);

    OptionSelectorFilterMatcher& operator=(OptionSelectorFilterMatcher&& other);

    ~OptionSelectorFilterMatcher() = default;

    OptionSelectorFilterMatcher& title(const std::string& title);

    OptionSelectorFilterMatcher& label(const std::string& label);

    OptionSelectorFilterMatcher& hasAtLeast(std::size_t minimum);

    OptionSelectorFilterMatcher& hasExactly(std::size_t amount);

    MatchResult match(const results::Filter& filter) const override;

    void match(MatchResult& matchResult, const results::Filter& filter) const override;

protected:
    struct _Priv;

    std::shared_ptr<_Priv> p;
};

}
}
}
