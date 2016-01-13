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

#include <QtGlobal>

namespace unity
{
namespace scopes
{
class Variant;
}
namespace scopeharness
{

namespace results
{
class Filter;
}

namespace matcher
{

class Q_DECL_EXPORT FilterMatcher
{
public:
    FilterMatcher(const std::string& id);

    FilterMatcher(const FilterMatcher& other);

    FilterMatcher(FilterMatcher&& other);

    FilterMatcher& operator=(const FilterMatcher& other);

    FilterMatcher& operator=(FilterMatcher&& other);

    ~FilterMatcher() = default;

    FilterMatcher& title(const std::string& title);

    virtual MatchResult match(const results::Filter& filter) const = 0;

    virtual void match(MatchResult& matchResult, const results::Filter& filter) const = 0;

protected:
    struct _Priv;

    std::shared_ptr<_Priv> p;
};

}
}
}
