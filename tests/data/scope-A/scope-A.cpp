/*
 * Copyright (C) 2013 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Michi Henning <michi.henning@canonical.com>
 */

#include <scopes/ScopeBase.h>
#include <scopes/Reply.h>
#include <scopes/Category.h>
#include <scopes/CategorisedResult.h>
#include <scopes/CategoryRenderer.h>

#include <iostream>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::api::scopes;

// Example scope A: replies synchronously to a query. (Replies are returned before returning from the run() method.)

class MyQuery : public QueryBase
{
public:
    MyQuery(string const& query) :
        query_(query)
    {
    }

    ~MyQuery() noexcept
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(ReplyProxy const& reply) override
    {
        if (query_ == "metadata")
        {
            auto cat = reply->register_category("cat1", "Category 1", "");
            CategorisedResult res(cat);
            res.set_uri("test:uri");
            res.set_title("result for: \"" + query_ + "\"");
            res.set_art("art");
            res.set_dnd_uri("test:dnd_uri");
            res.add_metadata("subtitle", Variant("subtitle"));
            res.add_metadata("emblem", Variant("emblem"));
            reply->push(res);
        }
        else if (query_ == "minimal")
        {
            CategoryRenderer minimal(R"({"schema-version": 1, "components": {"title": "title"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", minimal);
            CategorisedResult res(cat);
            res.set_uri("test:uri");
            res.set_title("result for: \"" + query_ + "\"");
            res.set_art("art");
            res.set_dnd_uri("test:dnd_uri");
            reply->push(res);
        }
        else
        {
            auto cat = reply->register_category("cat1", "Category 1", "");
            CategorisedResult res(cat);
            res.set_uri("test:uri");
            res.set_title("result for: \"" + query_ + "\"");
            res.set_art("art");
            res.set_dnd_uri("test:dnd_uri");
            reply->push(res);
        }
    }

private:
    string query_;
};

class MyScope : public ScopeBase
{
public:
    virtual int start(string const&, RegistryProxy const&) override
    {
        return VERSION;
    }

    virtual void stop() override {}

    virtual QueryBase::UPtr create_query(string const& q, VariantMap const&) override
    {
        QueryBase::UPtr query(new MyQuery(q));
        cout << "scope-A: created query: \"" << q << "\"" << endl;
        return query;
    }
};

extern "C"
{

    EXPORT
    unity::api::scopes::ScopeBase*
    // cppcheck-suppress unusedFunction
    UNITY_API_SCOPE_CREATE_FUNCTION()
    {
        return new MyScope;
    }

    EXPORT
    void
    // cppcheck-suppress unusedFunction
    UNITY_API_SCOPE_DESTROY_FUNCTION(unity::api::scopes::ScopeBase* scope_base)
    {
        delete scope_base;
    }

}
