/*
 * Copyright (C) 2015 Canonical, Ltd.
 *
 * Authors:
 *  Pawel Stolowski <pawel.stolowski@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "resultsmap.h"
#include <cassert>

ResultsMap::ResultsMap(QList<std::shared_ptr<unity::scopes::CategorisedResult>> &results)
{
    update(results, 0);
}

void ResultsMap::rebuild(QList<std::shared_ptr<unity::scopes::Result>> const &results)
{
    m_results.clear();
    int pos = 0;
    for (auto const& result: results) {
        assert(result);
        const ResultPos rpos { result, pos++ };
        m_results.insert({result->uri(), rpos });
    }
}

int ResultsMap::find(std::shared_ptr<unity::scopes::Result> const& result) const
{
    assert(result);
    auto it = m_results.find(result->uri());
    if (it != m_results.end()) {
        assert(it->second.result);
        while (it != m_results.end() && it->second.result->uri() == result->uri())
        {
            if (*(it->second.result) == *result) {
                return it->second.index;
            }
            ++it;
        }
    }
    return -1;
}

void ResultsMap::update(QList<std::shared_ptr<unity::scopes::CategorisedResult>> &results, int start)
{
    int pos = start;
    for (auto it = results.begin() + start; it != results.end(); ) {
        std::shared_ptr<unity::scopes::CategorisedResult> result = *it;
        assert(result);
        if (find(result) < 0) {
            const ResultPos rpos { result, pos++ };
            m_results.insert({result->uri(), rpos });
            ++it;
        } else {
            it = results.erase(it);
        }
    }
}

void ResultsMap::updateIndices(QList<std::shared_ptr<unity::scopes::Result>> const &results, int start, int end, int delta)
{
    for (int i = start; i<end; i++) {
        auto const result = results[i];
        auto it = m_results.find(result->uri());
        while (it != m_results.end() && it->second.result->uri() == result->uri())
        {
            if (*(it->second.result) == *result) {
                it->second.index += delta;
            }
            ++it;
        }
    }
}

void ResultsMap::clear()
{
    m_results.clear();
}
