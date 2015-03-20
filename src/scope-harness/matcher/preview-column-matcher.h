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

class PreviewMatcher;

class Q_DECL_EXPORT PreviewColumnMatcher final
{
public:
    PreviewColumnMatcher();

    PreviewColumnMatcher(const PreviewColumnMatcher& other);

    PreviewColumnMatcher(PreviewColumnMatcher&& other);

    ~PreviewColumnMatcher();

    PreviewColumnMatcher& operator=(const PreviewColumnMatcher& other);

    PreviewColumnMatcher& operator=(PreviewColumnMatcher&& other);

    PreviewColumnMatcher& column(const PreviewMatcher& previewMatcher);

    PreviewColumnMatcher& column(PreviewMatcher&& previewMatcher);

    MatchResult match(const std::vector<preview::PreviewWidgetList>& preview) const;

    void match(MatchResult& matchResult, const std::vector<preview::PreviewWidgetList>& preview) const;

protected:
    struct _Priv;

    std::shared_ptr<_Priv> p;
};

}
}
}
