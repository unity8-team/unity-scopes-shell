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

#ifndef NG_RESULTS_MAP_H
#define NG_RESULTS_MAP_H

#include <QList>
#include <memory>
#include <unity/scopes/CategorisedResult.h>
#include <map>

/**
  Helper class for Result -> row lookups, maintains a multimap internally
  allowing for duplicated Result uris.
*/
class ResultsMap
{
    public:
        ResultsMap() = default;

        // note: this constructor modifies the input results list (de-duplicates it).
        ResultsMap(QList<std::shared_ptr<unity::scopes::CategorisedResult>> &results);
        int find(std::shared_ptr<unity::scopes::Result> const& result) const;

        void rebuild(QList<std::shared_ptr<unity::scopes::Result>> &results);

        template <typename ResultType>
        void update(QList<std::shared_ptr<ResultType>> &results, int start)
        {
            int pos = start;
            for (auto it = results.begin() + start; it != results.end(); ) {
                std::shared_ptr<ResultType> result = *it;
                if (find(result) < 0) {
                    const ResultPos rpos { result, pos++ };
                    m_results.insert({result->uri(), rpos });
                    ++it;
                } else {
                    // remove duplicate from the input results array
                    it = results.erase(it);
                }
            }
        }

        void updateIndices(QList<std::shared_ptr<unity::scopes::Result>> const &results, int start, int end);
        void clear();
        void dump(QString const& msg);

    private:
        struct ResultPos {
            std::shared_ptr<unity::scopes::Result> result;
            int index;
        };
        std::multimap<std::string, ResultPos> m_results;
};

#endif
