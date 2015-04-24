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
        query_(query.query_string()),
        department_id_(query.department_id())
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
            CategoryRenderer rating_rndr(R"({"schema-version": 1, "components": {"title": "title", "attributes": "attributes"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", rating_rndr);
            CategorisedResult res(cat);
            res.set_uri("test:uri");
            res.set_title("result for: \"" + query_ + "\"");
            res.set_art("art");
            res.set_dnd_uri("test:dnd_uri");
            VariantBuilder attribute_builder;
            attribute_builder.add_tuple({{"value", Variant("21 reviews")}});
            res["attributes"] = attribute_builder.end();
            reply->push(res);
        }
        else if (query_ == "attributes")
        {
            CategoryRenderer rating_rndr(R"({"schema-version": 1, "components": {"title": "title", "attributes": {"field": "attributes", "max-count":3}}})");
            auto cat = reply->register_category("cat1", "Category 1", "", rating_rndr);
            CategorisedResult res(cat);
            res.set_uri("test:uri");
            res.set_title("result for: \"" + query_ + "\"");
            res.set_art("art");
            res.set_dnd_uri("test:dnd_uri");
            VariantBuilder attribute_builder;
            attribute_builder.add_tuple({{"value", Variant("21 reviews")}});
            attribute_builder.add_tuple({{"value", Variant("4 comments")}});
            attribute_builder.add_tuple({{"value", Variant("28 stars")}});
            attribute_builder.add_tuple({{"value", Variant("foobar")}});
            res["attributes"] = attribute_builder.end();
            reply->push(res);
        }
        else if (query_ == "background")
        {
            CategoryRenderer bkgr_rndr(R"({"schema-version": 1, "template": {"card-background": "color:///black"}, "components": {"title": "title", "background": "background"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", bkgr_rndr);
            CategorisedResult res(cat);
            res.set_uri("test:uri");
            res.set_title("result for: \"" + query_ + "\"");
            res.set_dnd_uri("test:dnd_uri");
            res["background"] = "gradient:///green/#ff00aa33";
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
        else if (query_ == "query")
        {
            CategoryRenderer minimal_rndr(R"({"schema-version": 1, "components": {"title": "title"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", minimal_rndr);
            CategorisedResult res(cat);
            res.set_uri("test:query");
            res.set_title("result for: \"" + query_ + "\"");
            reply->push(res);
        }
        else if (query_ == "expandable-widget")
        {
            CategoryRenderer minimal_rndr(R"({"schema-version": 1, "components": {"title": "title"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", minimal_rndr);
            CategorisedResult res(cat);
            res.set_uri("test:expandable-widget");
            res.set_art("art");
            res.set_title("result for: \"" + query_ + "\"");
            reply->push(res);
        }
        else if (query_ == "perform-query")
        {
            CategoryRenderer minimal_rndr(R"({"schema-version": 1, "components": {"title": "title"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", minimal_rndr);
            CategorisedResult res(cat);
            res.set_uri("scope://test:perform-query"); // FIXME: workaround for https://bugs.launchpad.net/ubuntu/+source/unity8/+bug/1428063
            res.set_title("result for: \"" + query_ + "\"");
            res["scope-id"] = "mock-scope-ttl";
            res.set_intercept_activation();
            reply->push(res);
        }
        else if (query_ == "perform-query2")
        {
            CategoryRenderer minimal_rndr(R"({"schema-version": 1, "components": {"title": "title"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", minimal_rndr);
            CategorisedResult res(cat);
            res.set_uri("scope://test:perform-query"); // FIXME: workaround for https://bugs.launchpad.net/ubuntu/+source/unity8/+bug/1428063
            res.set_title("result for: \"" + query_ + "\"");
            res["scope-id"] = "nonexisting-scope";
            res.set_intercept_activation();
            reply->push(res);
        }
        else if (query_ == "scope-uri")
        {
            CategoryRenderer minimal_rndr(R"({"schema-version": 1, "components": {"title": "title"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", minimal_rndr);
            CategorisedResult res(cat);
            res.set_uri("scope://mock-scope?q=next-scope-query");
            res.set_title("result for: \"" + query_ + "\"");
            reply->push(res);
        }
        else if (query_ == "next-scope-query")
        {
            CategoryRenderer minimal_rndr(R"({"schema-version": 1, "components": {"title": "title", "art": "art"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", minimal_rndr);
            CategorisedResult res(cat);
            res.set_uri("next-scope-query");
            res.set_art("next-scope-query-art");
            res.set_title("result for: \"" + query_ + "\"");
            reply->push(res);
        }
        else if (query_ == "two-categories")
        {
            auto cat1 = reply->register_category("cat1", "Category 1", "");
            auto cat2 = reply->register_category("cat2", "Category 2", "");
            CategorisedResult res1(cat1);
            res1.set_uri("test:uri");
            res1.set_title("result for: \"" + query_ + "\"");
            reply->push(res1);

            CategorisedResult res2(cat2);
            res2.set_uri("test:uri");
            res2.set_title("result for: \"" + query_ + "\"");
            reply->push(res2);
        }
        else if (query_ == "two-categories-reversed")
        {
            auto cat2 = reply->register_category("cat2", "Category 2", "");
            auto cat1 = reply->register_category("cat1", "Category 1", "");
            CategorisedResult res2(cat2);
            res2.set_uri("test:uri");
            res2.set_title("result for: \"" + query_ + "\"");
            reply->push(res2);

            CategorisedResult res1(cat1);
            res1.set_uri("test:uri");
            res1.set_title("result for: \"" + query_ + "\"");
            reply->push(res1);
        }
        else if (query_ == "two-categories-one-result")
        {
            auto cat1 = reply->register_category("cat1", "Category 1", "");
            auto cat2 = reply->register_category("cat2", "Category 2", "");
            CategorisedResult res1(cat1);
            res1.set_uri("test:uri");
            res1.set_title("result for: \"" + query_ + "\"");
            reply->push(res1);
        }
        else if (query_ == "expansion-query")
        {
            CategoryRenderer minimal_rndr(R"({"schema-version": 1, "components": {"title": "title"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", query(), minimal_rndr);
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
            res["session-id"] = search_metadata()["session-id"].get_string();
            res["query-id"] = Variant(search_metadata()["query-id"].get_int());
            res["booleanness"] = Variant(true);
            res.set_intercept_activation();
            reply->push(res);
        }
    }

private:
    string query_;
    string department_id_;
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
        if (result().uri().find("layout") != std::string::npos)
        {
            PreviewWidget w1("img", "image");
            w1.add_attribute_value("source", Variant("foo.png"));
            PreviewWidget w2("hdr", "header");
            w2.add_attribute_value("title", Variant("Preview title"));
            PreviewWidget w3("desc", "text");
            w3.add_attribute_value("text", Variant("Lorum ipsum..."));
            PreviewWidget w4("actions", "actions");

            VariantBuilder builder;
            builder.add_tuple({
                {"id", Variant("open")},
                {"label", Variant("Open")},
                {"uri", Variant("application:///tmp/non-existent.desktop")}
            });
            builder.add_tuple({
                {"id", Variant("download")},
                {"label", Variant("Download")}
            });
            builder.add_tuple({
                {"id", Variant("hide")},
                {"label", Variant("Hide")}
            });
            w4.add_attribute_value("actions", builder.end());

            ColumnLayout l1(1);
            l1.add_column({"img", "hdr", "desc", "actions", "extra"});
            ColumnLayout l2(2);
            l2.add_column({"img"});
            l2.add_column({"hdr", "desc", "actions"});

            reply->register_layout({l1, l2});
            PreviewWidgetList widgets({w1, w2, w3, w4});
            if (!scope_data_.is_null()) {
                PreviewWidget extra("extra", "text");
                extra.add_attribute_value("text", Variant("got scope data"));
                widgets.push_back(extra);
            }
            reply->push(widgets);
            return;
        }
        else if (result().uri().find("query") != std::string::npos)
        {
            PreviewWidget w1("actions", "actions");

            VariantBuilder builder;
            auto uri = CannedQuery("mock-scope").to_uri();
            builder.add_tuple({
                {"id", Variant("query")},
                {"label", Variant("Search")},
                {"uri", Variant(uri)}
            });
            w1.add_attribute_value("actions", builder.end());

            ColumnLayout l1(1);
            l1.add_column({"actions"});

            reply->register_layout({l1});
            PreviewWidgetList widgets({w1});
            reply->push(widgets);
            return;
        }
        else if (result().uri().find("expandable-widget") != std::string::npos)
        {
            PreviewWidget w1("exp", "expandable");
            w1.add_attribute_value("title", Variant("Expandable widget"));
            PreviewWidget w2("txt", "text");
            w2.add_attribute_value("title", Variant("Subwidget"));
            w2.add_attribute_value("text", Variant("Lorum ipsum"));
            PreviewWidget w3("img", "image");
            w3.add_attribute_mapping("source", "src");
            w1.add_widget(w2);
            w1.add_widget(w3);

            PreviewWidget w4("img", "image");
            w4.add_attribute_value("source", Variant("foo.png"));

            PreviewWidgetList widgets({w1, w4});
            reply->push(widgets);
            reply->push("src", Variant("bar.png"));
            return;
        }

        PreviewWidgetList widgets;
        PreviewWidget w1(R"({"id": "hdr", "type": "header", "components": {"title": "title", "subtitle": "uri", "attribute-1": "extra-data", "session-id": "session-id-val"}})");
        PreviewWidget w2(R"({"id": "img", "type": "image", "components": {"source": "art"}, "zoomable": false})");
        widgets.push_back(w1);
        widgets.push_back(w2);
        reply->push(widgets);

        reply->push("extra-data", Variant("foo"));
        reply->push("session-id-val", Variant(action_metadata()["session-id"]));
    }

private:
    Variant scope_data_;
};

class MyActivation : public ActivationQueryBase
{
public:
    MyActivation(Result const& result, ActionMetadata const& metadata) :
        ActivationQueryBase(result, metadata),
        status_(ActivationResponse::HideDash)
    {
    }

    MyActivation(Result const& result, ActionMetadata const& metadata, ActivationResponse::Status status) :
        ActivationQueryBase(result, metadata),
        status_(status)
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
            auto resp = ActivationResponse(CannedQuery(result()["scope-id"].get_string()));
            return resp;
        } else {
            auto resp = ActivationResponse(status_);
            resp.set_scope_data(extra_data_);
            return resp;
        }
    }

private:
    Variant extra_data_;
    ActivationResponse::Status status_;
};

class MyScope : public ScopeBase
{
public:
    virtual SearchQueryBase::UPtr search(CannedQuery const& q, SearchMetadata const& metadata) override
    {
        SearchQueryBase::UPtr query(new MyQuery(q, metadata));
        cout << "scope-A: created query: \"" << q.query_string() << "\"" << endl;
        return query;
    }

    virtual PreviewQueryBase::UPtr preview(Result const& result, ActionMetadata const& metadata) override
    {
        PreviewQueryBase::UPtr query(new MyPreview(result, metadata));
        cout << "scope-A: created preview query: \"" << result.uri() << "\"" << endl;
        return query;
    }

    virtual ActivationQueryBase::UPtr perform_action(Result const& result, ActionMetadata const& meta, std::string const& widget_id, std::string const& action_id)
    {
        if (widget_id == "actions" && action_id == "hide")
        {
            return ActivationQueryBase::UPtr(new MyActivation(result, meta));
        }
        else if (widget_id == "actions" && action_id == "download")
        {
            MyActivation* response = new MyActivation(result, meta, ActivationResponse::ShowPreview);
            response->setExtraData(meta.scope_data());
            return ActivationQueryBase::UPtr(response);
        }
        return ActivationQueryBase::UPtr(new MyActivation(result, meta, ActivationResponse::NotHandled));
    }

    virtual ActivationQueryBase::UPtr activate(Result const& result, ActionMetadata const& meta) override
    {
        if (result.uri().find("perform-query") != std::string::npos) {
            return ActivationQueryBase::UPtr(new MyActivation(result, meta, ActivationResponse::PerformQuery));
        }
        return ActivationQueryBase::UPtr(new MyActivation(result, meta));
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
