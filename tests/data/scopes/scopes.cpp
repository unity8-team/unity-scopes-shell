/*
 * Copyright (C) 2013 Canonical, Ltd.
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
 *  Michal Hruby <michal.hruby@canonical.com>
 */

#include <unity-scopes.h>

#include <iostream>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::scopes;

// Example scope A: replies synchronously to a query. (Replies are returned before returning from the run() method.)

class MyQuery : public SearchQueryBase
{
public:
    MyQuery(CannedQuery const& query, SearchMetadata const& metadata) :
        SearchQueryBase(query, metadata),
        query_(query.query_string())
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
        CategoryRenderer minimal_rndr(R"({"schema-version": 1, "components": {"title": "title"}})");
        auto cat = reply->register_category("cat1", "Category 1", "", minimal_rndr);
        CategorisedResult res(cat);
        res.set_uri("test:uri");
        res.set_title("result for: \"" + query_ + "\"");
        res.set_art("art");
        res.set_dnd_uri("test:dnd_uri");
        reply->push(res);
    }

private:
    string query_;
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

    virtual void run(PreviewReplyProxy const& reply) override
    {
        PreviewWidgetList widgets;
        PreviewWidget w1(R"({"id": "hdr", "type": "header", "components": {"title": "title", "subtitle": "uri"}})");
        PreviewWidget w2(R"({"id": "img", "type": "image", "components": {"source": "art"}, "zoomable": false})");
        widgets.push_back(w1);
        widgets.push_back(w2);
        reply->push(widgets);
    }

private:
    Variant scope_data_;
};

class MyScope : public ScopeBase
{
public:
    virtual SearchQueryBase::UPtr search(CannedQuery const& q, SearchMetadata const& metadata) override
    {
        SearchQueryBase::UPtr query(new MyQuery(q, metadata));
        cout << "scope-scopes: created query: \"" << q.query_string() << "\"" << endl;
        return query;
    }

    virtual PreviewQueryBase::UPtr preview(Result const& result, ActionMetadata const& metadata) override
    {
        PreviewQueryBase::UPtr query(new MyPreview(result, metadata));
        cout << "scope-scopes: created preview query: \"" << result.uri() << "\"" << endl;
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
