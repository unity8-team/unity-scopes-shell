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
    MyQuery(CannedQuery const& query, SearchMetadata const& metadata, VariantMap const& settings) :
        SearchQueryBase(query, metadata),
        query_(query.query_string()),
        department_id_(query.department_id()),
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
        else if (query_ == "layout" || query_ == "layout-push-order-change" || query_ == "layout-order-change" || query_ == "incomplete-layout"
                 || query_ == "preview-replace-with-removal" || query_ == "preview-replace-with-moves" || query_ == "preview-replace-no-layouts")
        {
            CategoryRenderer minimal_rndr(R"({"schema-version": 1, "components": {"title": "title"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", minimal_rndr);
            CategorisedResult res(cat);
            res.set_uri("test:" + query_);
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
        else if (query_ == "result-action")
        {
            CategoryRenderer minimal_rndr(R"({"schema-version": 1, "components": {"title": "title"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", minimal_rndr);
            CategorisedResult res(cat);

            res.set_uri("test:result-action");
            res.set_title("result for: \"" + query_ + "\"");

            // Add result actions
            VariantBuilder builder;
            builder.add_tuple({
                    {"id", Variant("action1")},
                    {"icon", Variant("icon1")},
                    {"label", Variant("Action1")},
            });
            builder.add_tuple({
                    {"id", Variant("action2")},
                    {"icon", Variant("icon2")},
                    {"label", Variant("Action2")},
            });
            res["social_attributes"] = builder.end(); // TODO: verify with shell
            reply->push(res);
        }
        else if (query_ == "update-preview")
        {
            CategoryRenderer minimal_rndr(R"({"schema-version": 1, "components": {"title": "title"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", minimal_rndr);
            CategorisedResult res(cat);
            res.set_uri("update-preview");
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
        else if (query_ == "settings-change")
        {
            auto cat = reply->register_category("cat1", "Category 1", "");
            CategorisedResult res(cat);
            res.set_uri("test:uri");
            res.set_title("result for: \"" + query_ + "\"");
            res.set_art("art");
            res["setting-distanceUnit"] = settings_["distanceUnit"];
            reply->push(res);
        }
        else if (query_ == "preview-with-duplicated-widget-id")
        {
            auto cat = reply->register_category("cat1", "Category 1", "");
            CategorisedResult res(cat);
            res.set_uri(query_);
            res.set_title("result for: \"" + query_ + "\"");
            res.set_art("art");
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

    virtual void run(PreviewReplyProxy const& reply) override
    {
        if (result().uri().find("incomplete-layout") != std::string::npos)
        {
            PreviewWidget w1("img", "image");
            w1.add_attribute_value("source", Variant("foo.png"));
            PreviewWidget w2("hdr", "header");
            w2.add_attribute_value("title", Variant("Preview title"));
            PreviewWidget w3("desc", "expandable");
            PreviewWidget w31("sub1", "text");
            PreviewWidget w32("sub2", "text");
            w3.add_widget(w31);
            w3.add_widget(w32);
            w3.add_attribute_value("text", Variant("Lorum ipsum..."));
            PreviewWidget w4("actions", "actions");

            VariantBuilder builder;
            builder.add_tuple({
                {"id", Variant("open")},
                {"label", Variant("Open")},
                {"uri", Variant("application:///tmp/non-existent.desktop")}
            });
            w4.add_attribute_value("actions", builder.end());

            ColumnLayout l1(1);
            l1.add_column({"img"});

            reply->register_layout({l1});
            PreviewWidgetList widgets({w2, w1, w3, w4});

            reply->push(widgets);
            return;
        }
        else if (result().uri().find("preview-with-duplicated-widget-id") != std::string::npos)
        {
            PreviewWidget w1("img", "image");
            PreviewWidget w2("author", "header");
            PreviewWidget w3("hdr", "header");
            PreviewWidget w4("author", "text");

            reply->push({w1, w2, w3, w4});
        }
        else if (result().uri().find("preview-replace-with-removal") != std::string::npos)
        {
            PreviewWidget w1("img", "image");
            PreviewWidget w2("hdr", "header");
            PreviewWidget w3("desc", "text");
            PreviewWidget w4("actions", "actions");

            VariantBuilder builder;
            builder.add_tuple({
                {"id", Variant("download")},
                {"label", Variant("Download")}
            });
            w4.add_attribute_value("actions", builder.end());

            ColumnLayout l1(1);
            PreviewWidgetList widgets;
            if (!scope_data_.is_null()) {
                PreviewWidget extra("extra", "text");
                extra.add_attribute_value("text", Variant("got scope data"));
                widgets = {w2, w4, extra};
                l1.add_column({"hdr", "actions", "extra"});
            } else {
                widgets = {w1, w2, w3, w4};
                l1.add_column({"img", "hdr", "desc", "actions"});
            }
            reply->register_layout({l1});
            reply->push(widgets);
            return;
        }
        else if (result().uri().find("preview-replace-no-layouts") != std::string::npos)
        {
            PreviewWidget w1("img", "image");
            PreviewWidget w2("hdr", "header");
            PreviewWidget w3("desc", "text");
            PreviewWidget w4("actions", "actions");

            VariantBuilder builder;
            builder.add_tuple({
                {"id", Variant("download")},
                {"label", Variant("Download")}
            });
            w4.add_attribute_value("actions", builder.end());
            reply->push({w1, w2, w3, w4});
            return;
        }
        else if (result().uri().find("preview-replace-with-moves") != std::string::npos)
        {
            PreviewWidget w1("img", "image");
            PreviewWidget w2("hdr", "header");
            PreviewWidget w3("desc", "text");
            PreviewWidget w4("actions", "actions");

            VariantBuilder builder;
            builder.add_tuple({
                {"id", Variant("download")},
                {"label", Variant("Download")}
            });
            w4.add_attribute_value("actions", builder.end());

            ColumnLayout l1(1);
            PreviewWidgetList widgets;
            if (!scope_data_.is_null()) {
                w1.add_attribute_value("source", Variant("foo.png"));
                widgets = {w1, w2, w3, w4};
                l1.add_column({"hdr", "desc", "actions", "img"});
            } else {
                w1.add_attribute_value("source", Variant("bar.png"));
                widgets = {w1, w2, w3, w4};
                l1.add_column({"img", "hdr", "desc", "actions"});
            }
            reply->register_layout({l1});
            reply->push(widgets);
            return;
        }
        else if (result().uri().find("layout-push-order-change") != std::string::npos)
        {
            PreviewWidget w1("img", "image");
            w1.add_attribute_value("source", Variant("foo2.png"));
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
                {"id", Variant("hideChanged")},
                {"label", Variant("Hide Changed")}
            });
            w4.add_attribute_value("actions", builder.end());

            ColumnLayout l1(1);
            l1.add_column({"img", "hdr", "desc", "actions", "extra"});
            ColumnLayout l2(2);
            l2.add_column({"img"});
            l2.add_column({"hdr", "desc", "actions"});

            reply->register_layout({l1, l2});
            PreviewWidgetList widgets({w3, w2, w1, w4}); // different push order
            if (!scope_data_.is_null()) {
                PreviewWidget extra("extra", "text");
                extra.add_attribute_value("text", Variant("got scope data"));
                widgets.push_back(extra);
            }
            reply->push(widgets);
            return;
        }
        else if (result().uri().find("layout-order-change") != std::string::npos)
        {
            PreviewWidget w1("img", "image");
            w1.add_attribute_value("source", Variant("foo2.png"));
            PreviewWidget w2("hdr", "header");
            w2.add_attribute_value("title", Variant("Preview title"));
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
                {"id", Variant("hideChanged")},
                {"label", Variant("Hide Changed")}
            });
            w4.add_attribute_value("actions", builder.end());

            ColumnLayout l1(1);
            l1.add_column({"hdr", "img", "actions", "extra"}); // hdr and img swapped, no desc
            ColumnLayout l2(2);
            l2.add_column({"img"});
            l2.add_column({"hdr", "actions"});

            reply->register_layout({l1, l2});
            PreviewWidgetList widgets({w1, w2, w4});
            if (!scope_data_.is_null()) {
                PreviewWidget extra("extra", "text");
                extra.add_attribute_value("text", Variant("got scope data"));
                widgets.push_back(extra);
            }
            reply->push(widgets);
            return;
        }
        else if (result().uri().find("layout") != std::string::npos)
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
                {"id", Variant("nothing")},
                {"label", Variant("Do nothing")}
            });
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
        else if (result().uri().find("update-preview") != std::string::npos)
        {
            PreviewWidget w1("icon-actions", "icon-actions");

            VariantBuilder builder;
            builder.add_tuple({
                {"id", Variant("dosomething1")},
                {"label", Variant("Do something 1")}
            });
            builder.add_tuple({
                {"id", Variant("dosomething2")},
                {"label", Variant("Do something 2")}
            });
            w1.add_attribute_value("actions", builder.end());

            ColumnLayout l1(1);
            l1.add_column({"icon-actions"});

            reply->register_layout({l1});
            PreviewWidgetList widgets({w1});
            reply->push(widgets);
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

    MyActivation(Result const& result, ActionMetadata const& metadata, ActivationResponse::Status status, std::string const& action_id) :
        ActivationQueryBase(result, metadata),
        status_(status),
        action_id_(action_id)
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
        if (status_ == ActivationResponse::Status::UpdateResult) {
            Result updatedRes(result());
            updatedRes["actionId"] = Variant(action_id_);
            return ActivationResponse(updatedRes);
        }
        else if (status_ == ActivationResponse::Status::PerformQuery) {
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
    std::string action_id_;
};


class UpdatePreviewWidgets : public ActivationQueryBase
{
public:
    UpdatePreviewWidgets(Result const& result, ActionMetadata const& metadata, PreviewWidgetList const &widgets)
        : ActivationQueryBase(result, metadata),
          widgets_(widgets)
    {
    }

    virtual ActivationResponse activate() override
    {
        ActivationResponse resp(widgets_);
        return resp;
    }

private:
    PreviewWidgetList widgets_;
    Variant extra_data_;
};

class MyScope : public ScopeBase
{
public:
    virtual SearchQueryBase::UPtr search(CannedQuery const& q, SearchMetadata const& metadata) override
    {
        SearchQueryBase::UPtr query(new MyQuery(q, metadata, settings()));
        cout << "scope-A: created query: \"" << q.query_string() << "\"" << endl;
        return query;
    }

    virtual PreviewQueryBase::UPtr preview(Result const& result, ActionMetadata const& metadata) override
    {
        PreviewQueryBase::UPtr query(new MyPreview(result, metadata));
        cout << "scope-A: created preview query: \"" << result.uri() << "\"" << endl;
        return query;
    }

    virtual ActivationQueryBase::UPtr perform_action(Result const& result, ActionMetadata const& meta, std::string const& widget_id, std::string const& action_id) override
    {
        cout << "scope-A: called perform_action: " << widget_id << ", " << action_id << endl;
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
        else if (widget_id == "icon-actions" && action_id == "dosomething1")
        {
            PreviewWidget w1("icon-actions", "icon-actions");

            VariantBuilder builder;
            builder.add_tuple({
                {"id", Variant("dosomething1")},
                {"label", Variant("Did something 1")}
            });
            builder.add_tuple({
                {"id", Variant("dosomething2")},
                {"label", Variant("Do something 2")}
            });
            w1.add_attribute_value("actions", builder.end());

            PreviewWidgetList widgets({w1});
            return ActivationQueryBase::UPtr(new UpdatePreviewWidgets(result, meta, widgets));
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

    ActivationQueryBase::UPtr activate_result_action(Result const& result, ActionMetadata const& meta, std::string const& action_id) override
    {
        return ActivationQueryBase::UPtr(new MyActivation(result, meta, ActivationResponse::UpdateResult, action_id));
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
