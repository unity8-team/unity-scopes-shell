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

#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/SearchReply.h>
#include <unity/scopes/PreviewReply.h>
#include <unity/scopes/ActivationBase.h>
#include <unity/scopes/Category.h>
#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/CategoryRenderer.h>
#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/ColumnLayout.h>
#include <unity/scopes/VariantBuilder.h>

#include <iostream>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::scopes;

// Example scope A: replies synchronously to a query. (Replies are returned before returning from the run() method.)

class MyQuery : public SearchQuery
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

    virtual void run(SearchReplyProxy const& reply) override
    {
        if (query_ == "metadata")
        {
            CategoryRenderer meta_rndr(R"({"schema-version": 1, "components": {"title": "title", "art": "art", "subtitle": "subtitle", "emblem": "icon", "mascot": "mascot"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", meta_rndr);
            CategorisedResult res(cat);
            res.set_uri("test:uri");
            res.set_title("result for: \"" + query_ + "\"");
            res.set_art("art");
            res.set_dnd_uri("test:dnd_uri");
            res["subtitle"] = "subtitle";
            res["icon"] = "emblem";
            reply->push(res);
        }
        else if (query_ == "rating")
        {
            CategoryRenderer rating_rndr(R"({"schema-version": 1, "components": {"title": "title", "rating": {"field": "rating", "type": "stars"}}})");
            auto cat = reply->register_category("cat1", "Category 1", "", rating_rndr);
            CategorisedResult res(cat);
            res.set_uri("test:uri");
            res.set_title("result for: \"" + query_ + "\"");
            res.set_art("art");
            res.set_dnd_uri("test:dnd_uri");
            res["rating"] = "***";
            reply->push(res);
        }
        else if (query_ == "minimal")
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
        else if (query_ == "music")
        {
            CategoryRenderer music_rndr(R"({"schema-version": 1, "components": {"title": "title", "art": "empty"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", music_rndr);
            CategorisedResult res(cat);
            res.set_uri("file:///tmp/foo.mp3");
            res.set_title("result for: \"" + query_ + "\"");
            res.set_dnd_uri("file:///tmp/foo.mp3");
            res["mimetype"] = "audio/mp3";
            res["artist"] = "Foo";
            res["album"] = "FooAlbum";
            reply->push(res);
        }
        else if (query_ == "layout")
        {
            CategoryRenderer minimal_rndr(R"({"schema-version": 1, "components": {"title": "title"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", minimal_rndr);
            CategorisedResult res(cat);
            res.set_uri("test:layout");
            res.set_title("result for: \"" + query_ + "\"");
            reply->push(res);
        }
        else if (query_ == "perform-query")
        {
            CategoryRenderer minimal_rndr(R"({"schema-version": 1, "components": {"title": "title"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", minimal_rndr);
            CategorisedResult res(cat);
            res.set_uri("test:perform-query");
            res.set_title("result for: \"" + query_ + "\"");
            res["scope-id"] = "mock-scope";
            res.set_intercept_activation();
            reply->push(res);
        }
        else if (query_ == "perform-query2")
        {
            CategoryRenderer minimal_rndr(R"({"schema-version": 1, "components": {"title": "title"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", minimal_rndr);
            CategorisedResult res(cat);
            res.set_uri("test:perform-query");
            res.set_title("result for: \"" + query_ + "\"");
            res["scope-id"] = "nonexisting-scope";
            res.set_intercept_activation();
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
            res.set_intercept_activation();
            reply->push(res);
        }
    }

private:
    string query_;
};

class MyPreview : public PreviewQuery
{
public:
    MyPreview(Result const& result, Variant const& scope_data = Variant()) :
        result_(result), scope_data_(scope_data)
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
        if (result_.uri().find("layout") != std::string::npos)
        {
            PreviewWidget w1("img", "image");
            w1.add_attribute("source", Variant("foo.png"));
            PreviewWidget w2("hdr", "header");
            w2.add_attribute("title", Variant("Preview title"));
            PreviewWidget w3("desc", "text");
            w3.add_attribute("text", Variant("Lorum ipsum..."));
            PreviewWidget w4("actions", "actions");

            VariantBuilder builder;
            builder.add_tuple({
                {"id", Variant("open")},
                {"label", Variant("Open")}
            });
            builder.add_tuple({
                {"id", Variant("download")},
                {"label", Variant("Download")}
            });
            w4.add_attribute("actions", builder.end());

            ColumnLayout l1(1);
            l1.add_column({"img", "hdr", "desc", "actions", "extra"});
            ColumnLayout l2(2);
            l2.add_column({"img"});
            l2.add_column({"hdr", "desc", "actions"});

            reply->register_layout({l1, l2});
            PreviewWidgetList widgets({w1, w2, w3, w4});
            if (!scope_data_.is_null()) {
                PreviewWidget extra("extra", "text");
                extra.add_attribute("text", Variant("got scope data"));
                widgets.push_back(extra);
            }
            reply->push(widgets);
            return;
        }

        PreviewWidgetList widgets;
        PreviewWidget w1(R"({"id": "hdr", "type": "header", "components": {"title": "title", "subtitle": "uri", "attribute-1": "extra-data"}})");
        PreviewWidget w2(R"({"id": "img", "type": "image", "components": {"source": "art"}, "zoomable": false})");
        widgets.push_back(w1);
        widgets.push_back(w2);
        reply->push(widgets);

        reply->push("extra-data", Variant("foo"));
    }

private:
    Result result_;
    Variant scope_data_;
};

class MyActivation : public ActivationBase
{
public:
    MyActivation(Result const& result) :
        result_(result), status_(ActivationResponse::HideDash)
    {
    }

    MyActivation(Result const& result, ActivationResponse::Status status) :
        result_(result), status_(status)
    {
    }

    ~MyActivation() noexcept
    {
    }

    void setExtraData(Variant const& extra)
    {
        extra_data_ = extra;
    }

    virtual void cancelled() override
    {
    }

    virtual ActivationResponse activate() override
    {
        if (status_ == ActivationResponse::Status::PerformQuery) {
            auto resp = ActivationResponse(Query(result_["scope-id"].get_string()));
            return resp;
        } else {
            auto resp = ActivationResponse(status_);
            if (extra_data_.which() == Variant::Dict) {
                resp.setHints(extra_data_.get_dict());
            }
            return resp;
        }
    }

private:
    Variant extra_data_;
    Result result_;
    ActivationResponse::Status status_;
};

class MyScope : public ScopeBase
{
public:
    virtual int start(string const&, RegistryProxy const&) override
    {
        return VERSION;
    }

    virtual void stop() override {}

    virtual QueryBase::UPtr create_query(Query const& q, SearchMetadata const&) override
    {
        QueryBase::UPtr query(new MyQuery(q.query_string()));
        cout << "scope-A: created query: \"" << q.query_string() << "\"" << endl;
        return query;
    }

    virtual QueryBase::UPtr preview(Result const& result, ActionMetadata const& metadata) override
    {
        QueryBase::UPtr query(new MyPreview(result, metadata.scope_data()));
        cout << "scope-A: created preview query: \"" << result.uri() << "\"" << endl;
        return query;
    }

    virtual ActivationBase::UPtr perform_action(Result const& result, ActionMetadata const& meta, std::string const& widget_id, std::string const& action_id)
    {
        if (widget_id == "actions" && action_id == "open")
        {
            return ActivationBase::UPtr(new MyActivation(result));
        }
        else if (widget_id == "actions" && action_id == "download")
        {
            MyActivation* response = new MyActivation(result, ActivationResponse::ShowPreview);
            response->setExtraData(meta.scope_data());
            return ActivationBase::UPtr(response);
        }
        return ActivationBase::UPtr(new MyActivation(result, ActivationResponse::NotHandled));
    }

    virtual ActivationBase::UPtr activate(Result const& result, ActionMetadata const&) override
    {
        if (result.uri().find("perform-query") != std::string::npos) {
            return ActivationBase::UPtr(new MyActivation(result, ActivationResponse::PerformQuery));
        }
        return ActivationBase::UPtr(new MyActivation(result));
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
