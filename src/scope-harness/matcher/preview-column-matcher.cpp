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
#include <scope-harness/matcher/preview-column-matcher.h>

#include <vector>

using namespace std;

namespace unity
{
namespace scopeharness
{
namespace matcher
{

struct PreviewColumnMatcher::Priv
{
    vector<PreviewMatcher> m_matchers;
};

PreviewColumnMatcher::PreviewColumnMatcher() :
        p(new Priv)
{
}

PreviewColumnMatcher::~PreviewColumnMatcher()
{
}

PreviewColumnMatcher::PreviewColumnMatcher(const PreviewColumnMatcher& other) :
        p(new Priv)
{
    *this = other;
}

PreviewColumnMatcher::PreviewColumnMatcher(PreviewColumnMatcher&& other)
{
    *this = move(other);
}

PreviewColumnMatcher& PreviewColumnMatcher::operator=(const PreviewColumnMatcher& other)
{
    p->m_matchers = other.p->m_matchers;
    return *this;
}

PreviewColumnMatcher& PreviewColumnMatcher::operator=(PreviewColumnMatcher&& other)
{
    p = move(other.p);
    return *this;
}

PreviewColumnMatcher& PreviewColumnMatcher::column(const PreviewMatcher& previewMatcher)
{
    p->m_matchers.emplace_back(previewMatcher);
    return *this;
}

PreviewColumnMatcher& PreviewColumnMatcher::column(PreviewMatcher&& previewMatcher)
{
    p->m_matchers.emplace_back(previewMatcher);
    return *this;
}

MatchResult PreviewColumnMatcher::match(const vector<preview::PreviewWidget::List>& preview) const
{
    MatchResult matchResult;
    match(matchResult, preview);
    return matchResult;
}

void PreviewColumnMatcher::match(MatchResult& matchResult, const vector<preview::PreviewWidget::List>& preview) const
{
    if (p->m_matchers.size() != preview.size())
    {
        matchResult.failure(
                "Incorrect number of preview columns "
                        + to_string(preview.size()) + ", expected "
                        + to_string(p->m_matchers.size()));
        return;
    }

    for (size_t i = 0; i < p->m_matchers.size(); ++i)
    {
        p->m_matchers.at(i).match(matchResult, preview.at(i));
    }
}

}
}
}
