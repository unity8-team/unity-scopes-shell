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
#include <scope-harness/preview/preview-widget-list.h>

namespace unity
{
namespace scopeharness
{
namespace matcher
{
class PreviewWidgetMatcher;

class Q_DECL_EXPORT PreviewMatcher final
{
public:
    PreviewMatcher();

    ~PreviewMatcher();

    PreviewMatcher(const PreviewMatcher& other);

    PreviewMatcher(PreviewMatcher&& other);

    PreviewMatcher& operator=(const PreviewMatcher& other);

    PreviewMatcher& operator=(PreviewMatcher&& other);

    PreviewMatcher& widget(const PreviewWidgetMatcher& previewWidgetMatcher);

    PreviewMatcher& widget(PreviewWidgetMatcher&& previewWidgetMatcher);

    MatchResult match(const preview::PreviewWidgetList& previewWidgetList) const;

    void match(MatchResult& matchResult, const preview::PreviewWidgetList& previewWidgetList) const;

protected:
    struct _Priv;

    std::shared_ptr<_Priv> p;
};

}
}
}
