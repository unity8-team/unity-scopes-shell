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
class Result;
}
namespace matcher
{

class Q_DECL_EXPORT ResultMatcher
{
public:
    ResultMatcher(const std::string& uri);

    ~ResultMatcher() = default;

    ResultMatcher(const ResultMatcher& other);

    ResultMatcher& operator=(const ResultMatcher& other);

    ResultMatcher& operator=(ResultMatcher&& other);

    ResultMatcher& dndUri(const std::string& dndUri);

    ResultMatcher& title(const std::string& title);

    ResultMatcher& art(const std::string& art);

    ResultMatcher& subtitle(const std::string& subtitle);

    ResultMatcher& emblem(const std::string& emblem);

    ResultMatcher& mascot(const std::string& mascot);

    ResultMatcher& attributes(const unity::scopes::Variant& attributes);

    ResultMatcher& summary(const unity::scopes::Variant& summary);

    ResultMatcher& background(const unity::scopes::Variant& background);

    ResultMatcher& property(const std::string& name, const unity::scopes::Variant& value);

    MatchResult match(const results::Result& result) const;

    void match(MatchResult& matchResult, const results::Result& result) const;

    std::string getUri() const;

protected:
    struct Priv;

    std::shared_ptr<Priv> p;
};

}
}
}
