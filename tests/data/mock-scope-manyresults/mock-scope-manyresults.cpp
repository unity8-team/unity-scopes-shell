/*
 * Copyright (C) 2015 Canonical, Ltd.
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
 *
 * Authors:
 *  Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <unity-scopes.h>

#include <iostream>
#include <thread>
#include <cstdlib>
#include <chrono>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::scopes;

// Example scope A: replies synchronously to a query. (Replies are returned before returning from the run() method.)

class MyQuery : public SearchQueryBase
{
public:
    MyQuery(CannedQuery const& query, SearchMetadata const& metadata, VariantMap const& settings) :
        SearchQueryBase(query, metadata),
        query_(query.query_string()),
        settings_(settings)
    {
    }

    ~MyQuery() noexcept
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        CategoryRenderer meta_rndr(R"({"schema-version": 1, "components": {"title": "title", "art": "art", "subtitle": "subtitle", "emblem": "icon", "mascot": "mascot"}})");
        auto cat1 = reply->register_category("cat1", "Category 1", "", meta_rndr);
        auto cat2 = reply->register_category("cat2", "Category 2", "", meta_rndr);

        if (query_ == "")
        {
            // 2000 items
            for (int i = 0; i<200; i++)
            {
                CategorisedResult res(cat1);
                res.set_uri("cat1_uri" + std::to_string(i));
                res.set_title("result5 for: \"" + query_ + "\"");
                reply->push(res);

                if (i % 100 == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(40));
                }
            }
        }
        else if (query_ == "search1")
        {
            // five results with uris 0..4 in a single category cat1
            for (int i = 0; i<5; i++) {
                CategorisedResult res(cat1);
                res.set_uri("cat1_uri" + std::to_string(i));
                res.set_title("result " + std::to_string(i) + " for: \"" + query_ + "\"");
                reply->push(res);
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        else if (query_ == "search2")
        {
            const int start = 3;
            // five results with uris 3..7 in categories cat1 and cat2
            for (int i = 0; i<5; i++) {
                {
                    CategorisedResult res(cat1);
                    res.set_uri("cat1_uri" + std::to_string(start + i));
                    res.set_title("result " + std::to_string(start + i) + " for: \"" + query_ + "\"");
                    reply->push(res);
                }
                {
                    CategorisedResult res(cat2);
                    res.set_uri("cat2_uri" + std::to_string(start + i));
                    res.set_title("result " + std::to_string(start + i) + " for: \"" + query_ + "\"");
                    reply->push(res);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
        }
        else if (query_ == "search3")
        {
            const int start = 3;
            // five results with uris 7..3 in categories cat1 and cat2
            for (int i = 4; i>=0; i--) {
                {
                    CategorisedResult res(cat1);
                    res.set_uri("cat1_uri" + std::to_string(start + i));
                    res.set_title("result " + std::to_string(start + i) + " for: \"" + query_ + "\"");
                    reply->push(res);
                }
                {
                    CategorisedResult res(cat2);
                    res.set_uri("cat2_uri" + std::to_string(start + i));
                    res.set_title("result " + std::to_string(start + i) + " for: \"" + query_ + "\"");
                    reply->push(res);
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(30));
            }
        }
        else if (query_ == "search4")
        {
            // one result with uri 5 in cat2
            {
                CategorisedResult res(cat2);
                res.set_uri("cat2_uri5");
                res.set_title("result5 for: \"" + query_ + "\"");
                reply->push(res);
            }
        }
        else if (query_ == "search5")
        {
            for (int i = 0; i<2; i++) {
                CategorisedResult res(cat2);
                res.set_uri("cat2_uri" + std::to_string(i));
                res.set_title("result");
                reply->push(res);
            }
        }
        else if (query_ == "search6")
        {
            // 2 items, reversed order of search5
            for (int i = 1; i>=0; i--) {
                CategorisedResult res(cat2);
                res.set_uri("cat2_uri" + std::to_string(i));
                res.set_title("result");
                reply->push(res);
            }
        }
        else if (query_ == "lots_of_results")
        {
            // 2000 items
            for (int i = 0; i<2000; i++)
            {
                CategorisedResult res(cat1);
                res.set_uri("cat1_uri" + std::to_string(i));
                res.set_title("result5 for: \"" + query_ + "\"");
                reply->push(res);

                if (i % 100 == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(40));
                }
            }
        }
        else if (query_ == "lots_of_results_reversed")
        {
            // 2000 items, in the reversed order of search5
            for (int i = 1999; i>=0; i--)
            {
                CategorisedResult res(cat1);
                res.set_uri("cat1_uri" + std::to_string(i));
                res.set_title("result5 for: \"" + query_ + "\"");
                reply->push(res);

                if (i % 100 == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(40));
                }
            }
        }
        else if (query_ == "lots_of_results_reversed_plus_some")
        {
            // 2100 items, in the reversed order of search5, plus 100 extra results
            for (int i = 2099; i>=0; i--)
            {
                CategorisedResult res(cat1);
                res.set_uri("cat1_uri" + std::to_string(i));
                res.set_title("result5 for: \"" + query_ + "\"");
                reply->push(res);

                if (i % 100 == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(40));
                }
            }
        }
        else if (query_ == "lots_of_results_half_of_them_missing")
        {
            // 1000 items with uris matching every other item from previous search
            for (int i = 0; i<1000; i++)
            {
                CategorisedResult res(cat1);
                res.set_uri("cat1_uri" + std::to_string(i*2));
                res.set_title("result5 for: \"" + query_ + "\"");
                reply->push(res);

                if (i % 100 == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(40));
                }
            }
        }
        else if (query_ == "lots_of_results_2")
        {
            // 2000 items, all different than in previous searches
            for (int i = 0; i<2000; i++)
            {
                CategorisedResult res(cat1);
                res.set_uri("cat1_uri" + std::to_string(5000 + i));
                res.set_title("result5 for: \"" + query_ + "\"");
                reply->push(res);

                if (i % 100 == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(40));
                }
            }
        }
        else if (query_ == "lots_of_results_fast")
        {
            // 100 items
            for (int i = 0; i<10; i++)
            {
                CategorisedResult res(cat1);
                res.set_uri("cat1_uri" + std::to_string(i));
                res.set_title("result5 for: \"" + query_ + "\"");
                reply->push(res);
            }
        }
        else if (query_ == "lots_of_results_reversed_fast")
        {
            // 100 items, in the reversed order of search5
            for (int i = 9; i>=0; i--)
            {
                CategorisedResult res(cat1);
                res.set_uri("cat1_uri" + std::to_string(i));
                res.set_title("result5 for: \"" + query_ + "\"");
                reply->push(res);
            }
        }
        else if (query_ == "duplicated_uris1")
        {
            for (int i = 0; i<10; i++)
            {
                CategorisedResult res(cat1);
                res.set_uri("uri");
                res.set_title("result " + std::to_string(i));
                reply->push(res);
            }
        }
        else if (query_ == "duplicated_uris2")
        {
            for (int i = 0; i<10; i++)
            {
                CategorisedResult res(cat1);
                if (i % 2 == 0) { // every other result uses same uri
                    res.set_uri("uri");
                } else {
                    res.set_uri("uri" + std::to_string(i));
                }
                res.set_title("result " + std::to_string(i));
                reply->push(res);
            }
        }
        else if (query_ == "duplicated_results")
        {
            for (int i = 0; i<2; i++)
            {
                CategorisedResult res(cat1);
                res.set_uri("uri");
                res.set_title("result");
                reply->push(res);
            }
        }
        else if (query_ == "two-categories")
        {
            auto cat3 = reply->register_category("cat3", "Category 3", "", meta_rndr);
            for (int i = 0; i<10; i++)
            {
                CategorisedResult res(cat1);
                res.set_uri("uri" + std::to_string(i));
                res.set_title("result " + std::to_string(i));
                reply->push(res);

                CategorisedResult res2(cat3);
                res2.set_uri("uri" + std::to_string(i));
                res2.set_title("result " + std::to_string(i));
                reply->push(res2);
            }
        }
        else if (query_ == "two-categories-second-gone")
        {
            for (int i = 0; i<10; i++)
            {
                CategorisedResult res(cat1);
                res.set_uri("uri" + std::to_string(i));
                res.set_title("result " + std::to_string(i));
                reply->push(res);
            }
        }
        else if (query_.find("random") == 0) // "random", followed by an int for the number of results
        {
            auto rand_start = rand() % 30;
            for (int i = 0; i<std::stoi(query_.substr(6)); i++)
            {
                CategorisedResult res(cat1);
                res.set_uri("cat1_uri" + std::to_string(rand_start + i));
                res.set_title("result" + std::to_string(i));
                reply->push(res);
                if (i % 2 == 0) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(40));
                }
            }
        }
    }

private:
    string query_;
    VariantMap settings_;
};

class MyPreview : public PreviewQueryBase
{
public:
    MyPreview(Result const& result, ActionMetadata const& metadata) :
        PreviewQueryBase(result, metadata),
        scope_data_(metadata.scope_data())
    {
    }

    ~MyPreview() noexcept
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(PreviewReplyProxy const&) override
    {
    }

private:
    Variant scope_data_;
};

class MyScope : public ScopeBase
{
public:
    virtual SearchQueryBase::UPtr search(CannedQuery const& q, SearchMetadata const& metadata) override
    {
        SearchQueryBase::UPtr query(new MyQuery(q, metadata, settings()));
        return query;
    }

    virtual PreviewQueryBase::UPtr preview(Result const& result, ActionMetadata const& metadata) override
    {
        PreviewQueryBase::UPtr query(new MyPreview(result, metadata));
        return query;
    }
};

extern "C"
{

    EXPORT
    unity::scopes::ScopeBase*
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_CREATE_FUNCTION()
    {
        return new MyScope;
    }

    EXPORT
    void
    // cppcheck-suppress unusedFunction
    UNITY_SCOPE_DESTROY_FUNCTION(unity::scopes::ScopeBase* scope_base)
    {
        delete scope_base;
    }

}
