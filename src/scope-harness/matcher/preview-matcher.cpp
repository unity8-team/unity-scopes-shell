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

#include <scope-harness/matcher/preview-matcher.h>
#include <scope-harness/matcher/preview-widget-matcher.h>

using namespace std;

namespace unity
{
namespace scopeharness
{
namespace matcher
{

struct PreviewMatcher::Priv
{
    vector<PreviewWidgetMatcher> m_matchers;
};

PreviewMatcher::PreviewMatcher() :
        p(new Priv)
{
}

PreviewMatcher::~PreviewMatcher()
{
}

PreviewMatcher::PreviewMatcher(const PreviewMatcher& other) :
        p(new Priv)
{
    *this = other;
}

PreviewMatcher::PreviewMatcher(PreviewMatcher&& other)
{
    *this = move(other);
}

PreviewMatcher& PreviewMatcher::operator=(const PreviewMatcher& other)
{
    p->m_matchers = other.p->m_matchers;
    return *this;
}

PreviewMatcher& PreviewMatcher::operator=(PreviewMatcher&& other)
{
    p = move(other.p);
    return *this;
}

PreviewMatcher& PreviewMatcher::widget(const PreviewWidgetMatcher& previewWidgetMatcher)
{
    p->m_matchers.emplace_back(previewWidgetMatcher);
    return *this;
}

PreviewMatcher& PreviewMatcher::widget(PreviewWidgetMatcher&& previewWidgetMatcher)
{
    p->m_matchers.emplace_back(previewWidgetMatcher);
    return *this;
}

MatchResult PreviewMatcher::match(const preview::PreviewWidget::List& previewWidgetList) const
{
    MatchResult matchResult;
    match(matchResult, previewWidgetList);
    return matchResult;
}

void PreviewMatcher::match(MatchResult& matchResult, const preview::PreviewWidget::List& previewWidgetList) const
{
    if (p->m_matchers.size() != previewWidgetList.size())
    {
        matchResult.failure(
                "Incorrect number of preview widgets "
                        + to_string(previewWidgetList.size()) + ", expected "
                        + to_string(p->m_matchers.size()));
        return;
    }

    for (size_t i = 0; i < p->m_matchers.size(); ++i)
    {
        p->m_matchers.at(i).match(matchResult, previewWidgetList.at(i));
    }
}

}
}
}
