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

#pragma once

#include <scope-harness/category.h>

#include <memory>
#include <string>

#include <qglobal.h>

namespace unity
{
namespace scopeharness
{

class CategoryMatcher;
class ResultsView;

class Q_DECL_EXPORT CategoryListMatcher
{
public:
    enum class Mode
    {
        all, by_id, starts_with
    };

    CategoryListMatcher();

    ~CategoryListMatcher() = default;

    CategoryListMatcher& mode(Mode mode);

    CategoryListMatcher& category(const CategoryMatcher& categoryMatcher);

    CategoryListMatcher& category(CategoryMatcher&& categoryMatcher);

    CategoryListMatcher& hasAtLeast(unsigned int minimum);

    CategoryListMatcher& hasExactly(unsigned int amount);

    MatchResult match(const Category::List& resultList) const;

protected:
    CategoryListMatcher(const CategoryListMatcher& other) = delete;

    CategoryListMatcher& operator=(const CategoryListMatcher& other) = delete;

    CategoryListMatcher& operator=(CategoryListMatcher&& other) = delete;

    struct Priv;

    std::shared_ptr<Priv> p;
};

}
}
