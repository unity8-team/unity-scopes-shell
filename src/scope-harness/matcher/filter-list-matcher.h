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

#include <scope-harness/view/filters-view.h>
#include <scope-harness/matcher/match-result.h>

#include <memory>
#include <string>

#include <QtGlobal>

namespace unity
{
namespace scopeharness
{
namespace matcher
{

class FilterMatcher;

class Q_DECL_EXPORT FilterListMatcher final
{
public:
    enum class Mode
    {
        all, by_id, starts_with
    };

    FilterListMatcher();

    ~FilterListMatcher() = default;

    FilterListMatcher& mode(Mode mode);

    FilterListMatcher& filter(const FilterMatcher& filterMatcher);

    FilterListMatcher& filter(FilterMatcher&& filterMatcher);

    FilterListMatcher& hasAtLeast(std::size_t minimum);

    FilterListMatcher& hasExactly(std::size_t amount);

    MatchResult match(const view::FiltersView::SPtr& filterList) const;

protected:
    FilterListMatcher(const FilterListMatcher& other) = delete;

    FilterListMatcher(FilterListMatcher&& other) = delete;

    FilterListMatcher& operator=(const FilterListMatcher& other) = delete;

    FilterListMatcher& operator=(FilterListMatcher&& other) = delete;

    struct _Priv;

    std::shared_ptr<_Priv> p;
};

}
}
}
