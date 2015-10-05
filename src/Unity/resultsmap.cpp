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

ResultsMap::ResultsMap(QList<std::shared_ptr<unity::scopes::Result>> const &results)
{
    int pos = 0;
    for (auto result: results) {
        m_results.insert({result->uri(), { result, pos++} });
    }
}

ResultsMap::ResultsMap(QList<std::shared_ptr<unity::scopes::CategorisedResult>> const &results)
{
    int pos = 0;
    for (auto result: results) {
        m_results.insert({result->uri(), { std::dynamic_pointer_cast<unity::scopes::Result>(result), pos++} });
    }
}

int ResultsMap::find(std::shared_ptr<unity::scopes::Result> const& result) const
{
    auto it = m_results.find(result->uri());
    if (it != m_results.end()) {
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
