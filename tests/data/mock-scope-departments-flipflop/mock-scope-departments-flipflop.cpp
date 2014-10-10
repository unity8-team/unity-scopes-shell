/*
 * Copyright (C) 2014 Canonical Ltd
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
 * Authored by: Pete Woods <pete.woods@canonical.com>
 */

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/SearchReply.h>

#include <iostream>
#include <thread>
#include <atomic>
#include <sstream>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::scopes;

class MyQuery : public SearchQueryBase
{
public:
    MyQuery(CannedQuery const& query, SearchMetadata const& metadata, bool flipflop) :
        SearchQueryBase(query, metadata),
        flipflop_(flipflop)
    {
    }

    ~MyQuery()
    {
    }

    virtual void cancelled() override
    {
    }

    Department::SPtr create_root_dep(CannedQuery const& query)
    {
        Department::SPtr child_dep;
        Department::SPtr root_dep;
        root_dep = Department::create("", query, "All departments");

        child_dep = Department::create("books", query, "Books");
        child_dep->set_has_subdepartments();
        root_dep->add_subdepartment(child_dep);

        child_dep = Department::create("movies", query, "Movies, TV, Music");
        child_dep->set_has_subdepartments();
        root_dep->add_subdepartment(child_dep);

        if (flipflop_)
        {
            child_dep = Department::create("electronics", query, "Electronics");
            child_dep->set_has_subdepartments();
            root_dep->add_subdepartment(child_dep);
        }

        child_dep = Department::create("home", query, "Home, Garden & DIY");
        child_dep->set_has_subdepartments();
        root_dep->add_subdepartment(child_dep);

        child_dep = Department::create("toys", query, "Toys, Children & Baby");
        child_dep->set_has_subdepartments();
        root_dep->add_subdepartment(child_dep);

        return root_dep;
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        Department::SPtr child_dep;
        Department::SPtr root_dep;
        Department::SPtr active_dep;

        root_dep = create_root_dep(query());

        reply->register_departments(root_dep);

        auto cat1 = reply->register_category("cat1", "Category 1", "");
        CategorisedResult res1(cat1);
        res1.set_uri("test:uri");
        res1.set_title("result for: \"" + query().query_string() + "\"");
        reply->push(res1);
    }

protected:
    bool flipflop_;
};

class MyScope : public ScopeBase
{
public:
    MyScope()
    {
        flipflop_ = false;
    }

    virtual SearchQueryBase::UPtr search(CannedQuery const& q, SearchMetadata const& metadata) override
    {
        flipflop_ = !flipflop_;
        return SearchQueryBase::UPtr(new MyQuery(q, metadata, flipflop_));
    }

    virtual PreviewQueryBase::UPtr preview(Result const&, ActionMetadata const&) override
    {
        return nullptr;
    }

    std::atomic<bool> flipflop_;
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
