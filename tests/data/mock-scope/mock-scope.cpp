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
    MyQuery(string const& query, string const& department_id) :
        query_(query), department_id_(department_id)
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
            CategoryRenderer rating_rndr(R"({"schema-version": 1, "components": {"title": "title", "attributes": "attributes"}})");
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
        else if (query_ == "scope-uri")
        {
            CategoryRenderer minimal_rndr(R"({"schema-version": 1, "components": {"title": "title"}})");
            auto cat = reply->register_category("cat1", "Category 1", "", minimal_rndr);
            CategorisedResult res(cat);
            res.set_uri("scope://mock-scope?q=next-scope-uri");
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
        else if (query_ == "dep-query")
        {
            Department::SPtr child_dep;
            Department::SPtr root_dep;
            Department::SPtr active_dep;
            CannedQuery query("mock-scope", "dep-query", "");
            root_dep = Department::create("", query, "All departments");

            child_dep = Department::create("books", query, "Books");
            child_dep->set_has_subdepartments();
            root_dep->add_subdepartment(child_dep);

            if (department_id_.compare(0, 5, "books") == 0)
            {
                active_dep = child_dep;
                child_dep = Department::create("books-kindle", query, "Kindle Books");
                active_dep->add_subdepartment(child_dep);

                child_dep = Department::create("books-study", query, "Books for Study");
                active_dep->add_subdepartment(child_dep);

                child_dep = Department::create("books-audio", query, "Audiobooks");
                active_dep->add_subdepartment(child_dep);

                // and this is the only leaf department for which the scope provides good data
                if (department_id_ == "books-audio") active_dep = child_dep;
            }

            child_dep = Department::create("movies", query, "Movies, TV, Music");
            child_dep->set_has_subdepartments();
            root_dep->add_subdepartment(child_dep);

            child_dep = Department::create("electronics", query, "Electronics");
            child_dep->set_has_subdepartments();
            root_dep->add_subdepartment(child_dep);

            child_dep = Department::create("home", query, "Home, Garden & DIY");
            child_dep->set_has_subdepartments();
            root_dep->add_subdepartment(child_dep);

            if (department_id_.compare(0, 4, "home") == 0)
            {
                active_dep = child_dep;
                child_dep = Department::create("home-garden", query, "Garden & Outdoors");
                active_dep->add_subdepartment(child_dep);

                child_dep = Department::create("home-furniture", query, "Homeware & Furniture");
                active_dep->add_subdepartment(child_dep);

                child_dep = Department::create("home-kitchen", query, "Kitchen & Dining");
                active_dep->add_subdepartment(child_dep);
            }

            child_dep = Department::create("toys", query, "Toys, Children & Baby");
            child_dep->set_has_subdepartments();
            root_dep->add_subdepartment(child_dep);

            if (department_id_.compare(0, 4, "toys") == 0)
            {
                active_dep = child_dep;
                child_dep = Department::create("toys-games", query, "Toys & Games");
                active_dep->add_subdepartment(child_dep);

                child_dep = Department::create("toys-baby", query, "Baby");
                active_dep->add_subdepartment(child_dep);
            }

            if (!active_dep)
            {
                active_dep = root_dep;
            }

            reply->register_departments(root_dep, active_dep);

            auto cat1 = reply->register_category("cat1", "Category 1", "");
            CategorisedResult res1(cat1);
            res1.set_uri("test:uri");
            res1.set_title("result for: \"" + query_ + "\"");
            reply->push(res1);
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
    string department_id_;
};

class MyPreview : public PreviewQueryBase
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

class MyActivation : public ActivationQueryBase
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
            auto resp = ActivationResponse(CannedQuery(result_["scope-id"].get_string()));
            return resp;
        } else {
            auto resp = ActivationResponse(status_);
            resp.set_scope_data(extra_data_);
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

    virtual SearchQueryBase::UPtr search(CannedQuery const& q, SearchMetadata const&) override
    {
        SearchQueryBase::UPtr query(new MyQuery(q.query_string(), q.department_id()));
        cout << "scope-A: created query: \"" << q.query_string() << "\"" << endl;
        return query;
    }

    virtual PreviewQueryBase::UPtr preview(Result const& result, ActionMetadata const& metadata) override
    {
        PreviewQueryBase::UPtr query(new MyPreview(result, metadata.scope_data()));
        cout << "scope-A: created preview query: \"" << result.uri() << "\"" << endl;
        return query;
    }

    virtual ActivationQueryBase::UPtr perform_action(Result const& result, ActionMetadata const& meta, std::string const& widget_id, std::string const& action_id)
    {
        if (widget_id == "actions" && action_id == "hide")
        {
            return ActivationQueryBase::UPtr(new MyActivation(result));
        }
        else if (widget_id == "actions" && action_id == "download")
        {
            MyActivation* response = new MyActivation(result, ActivationResponse::ShowPreview);
            response->setExtraData(meta.scope_data());
            return ActivationQueryBase::UPtr(response);
        }
        return ActivationQueryBase::UPtr(new MyActivation(result, ActivationResponse::NotHandled));
    }

    virtual ActivationQueryBase::UPtr activate(Result const& result, ActionMetadata const&) override
    {
        if (result.uri().find("perform-query") != std::string::npos) {
            return ActivationQueryBase::UPtr(new MyActivation(result, ActivationResponse::PerformQuery));
        }
        return ActivationQueryBase::UPtr(new MyActivation(result));
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
