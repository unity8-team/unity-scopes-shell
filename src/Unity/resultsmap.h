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
#define NGRESULTS_MAP_H

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
        ResultsMap(QList<std::shared_ptr<unity::scopes::Result>> const &results);
        ResultsMap(QList<std::shared_ptr<unity::scopes::CategorisedResult>> const &results);
        int find(std::shared_ptr<unity::scopes::Result> const& result) const;

    private:
        struct ResultPos {
            std::shared_ptr<unity::scopes::Result> result;
            int index;
        };
        std::multimap<std::string, ResultPos> m_results;
};

#endif
