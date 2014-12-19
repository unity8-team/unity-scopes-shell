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
namespace preview
{
class PreviewWidget;
}
namespace matcher
{

class Q_DECL_EXPORT PreviewWidgetMatcher final
{
public:
    PreviewWidgetMatcher(const std::string& id);

    PreviewWidgetMatcher(const PreviewWidgetMatcher& other);

    PreviewWidgetMatcher(PreviewWidgetMatcher&& other);

    PreviewWidgetMatcher& operator=(const PreviewWidgetMatcher& other);

    PreviewWidgetMatcher& operator=(PreviewWidgetMatcher&& other);

    ~PreviewWidgetMatcher();

    PreviewWidgetMatcher& type(const std::string& type);

    PreviewWidgetMatcher& data(const unity::scopes::Variant& data);

    PreviewWidgetMatcher& data(unity::scopes::Variant&& data);

    MatchResult match(const preview::PreviewWidget& previewWidget) const;

    void match(MatchResult& matchResult, const preview::PreviewWidget& previewWidget) const;

protected:
    struct Priv;

    std::shared_ptr<Priv> p;
};

}
}
}
