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
class Category;
}
namespace matcher
{
class ResultMatcher;

class Q_DECL_EXPORT CategoryMatcher
{
public:
    enum class Mode
    {
        all, starts_with, by_uri
    };

    CategoryMatcher(const std::string& id);

    CategoryMatcher(const CategoryMatcher& other);

    CategoryMatcher(CategoryMatcher&& other);

    CategoryMatcher& operator=(const CategoryMatcher& other);

    CategoryMatcher& operator=(CategoryMatcher&& other);

    ~CategoryMatcher() = default;

    CategoryMatcher& mode(Mode mode);

    CategoryMatcher& title(const std::string& title);

    CategoryMatcher& icon(const std::string& icon);

    CategoryMatcher& headerLink(const std::string& headerLink);

    CategoryMatcher& hasAtLeast(std::size_t minimum);

    CategoryMatcher& renderer(const unity::scopes::Variant& renderer);

    CategoryMatcher& components(const unity::scopes::Variant& components);

    CategoryMatcher& result(const ResultMatcher& resultMatcher);

    CategoryMatcher& result(ResultMatcher&& resultMatcher);

    std::string& getId() const;

    MatchResult match(const results::Category& category) const;

    void match(MatchResult& matchResult, const results::Category& category) const;

protected:
    struct Priv;

    std::shared_ptr<Priv> p;
};

}
}
}
