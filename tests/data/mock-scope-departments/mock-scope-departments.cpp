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
    MyQuery(CannedQuery const& query, SearchMetadata const& metadata) :
        SearchQueryBase(query, metadata),
        department_id_(query.department_id())
    {
    }

    ~MyQuery()
    {
    }

    virtual void cancelled() override
    {
    }

    static Department::SPtr create_root_dep(CannedQuery const& query)
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

        child_dep = Department::create("electronics", query, "Electronics");
        child_dep->set_has_subdepartments();
        root_dep->add_subdepartment(child_dep);

        child_dep = Department::create("home", query, "Home, Garden & DIY");
        child_dep->set_has_subdepartments();
        root_dep->add_subdepartment(child_dep);

        child_dep = Department::create("toys", query, "Toys, Children & Baby");
        child_dep->set_has_subdepartments();
        root_dep->add_subdepartment(child_dep);

        return root_dep;
    }

    static Department::SPtr get_department_by_id(Department::SPtr root_dep, std::string const& dep_id)
    {
        auto children = root_dep->subdepartments();
        for (auto it = children.begin(); it != children.end(); ++it)
        {
            if ((*it)->id() == dep_id) return const_pointer_cast<Department>(*it);
        }
        return Department::SPtr();
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        Department::SPtr child_dep;
        Department::SPtr root_dep;
        Department::SPtr active_dep;

        root_dep = create_root_dep(query());

        if (department_id_.compare(0, 5, "books") == 0)
        {
            active_dep = get_department_by_id(root_dep, "books");
            child_dep = Department::create("books-kindle", query(), "Kindle Books");
            active_dep->add_subdepartment(child_dep);

            child_dep = Department::create("books-study", query(), "Books for Study");
            active_dep->add_subdepartment(child_dep);

            child_dep = Department::create("books-audio", query(), "Audiobooks");
            active_dep->add_subdepartment(child_dep);
        }

        if (department_id_.compare(0, 4, "home") == 0)
        {
            active_dep = get_department_by_id(root_dep, "home");
            child_dep = Department::create("home-garden", query(), "Garden & Outdoors");
            active_dep->add_subdepartment(child_dep);

            child_dep = Department::create("home-furniture", query(), "Homeware & Furniture");
            active_dep->add_subdepartment(child_dep);

            child_dep = Department::create("home-kitchen", query(), "Kitchen & Dining");
            active_dep->add_subdepartment(child_dep);
        }

        if (department_id_.compare(0, 4, "toys") == 0)
        {
            active_dep = get_department_by_id(root_dep, "toys");
            child_dep = Department::create("toys-games", query(), "Toys & Games");
            active_dep->add_subdepartment(child_dep);

            child_dep = Department::create("toys-baby", query(), "Baby");
            active_dep->add_subdepartment(child_dep);
        }

        // provide only partial tree for this leaf
        if (department_id_ == "toys-games")
        {
            root_dep = Department::create("", query(), "All departments");
            child_dep = Department::create("toys", query(), "Toys, Children & Baby");
            root_dep->add_subdepartment(child_dep);
            active_dep = Department::create("toys-games", query(), "Toys & Games");
            child_dep->add_subdepartment(active_dep);
        }

        reply->register_departments(root_dep);

        auto cat1 = reply->register_category("cat1", "Category 1", "");
        CategorisedResult res1(cat1);
        res1.set_uri("test:uri");
        res1.set_title("result for: \"" + query().query_string() + "\", department \"" + department_id_ + "\"");
        reply->push(res1);
    }

protected:
    string department_id_;
};

class MyScope : public ScopeBase
{
public:
    MyScope()
    {
    }

    virtual SearchQueryBase::UPtr search(CannedQuery const& q, SearchMetadata const& metadata) override
    {
        return SearchQueryBase::UPtr(new MyQuery(q, metadata));
    }

    virtual PreviewQueryBase::UPtr preview(Result const&, ActionMetadata const&) override
    {
        return nullptr;
    }

    virtual ChildScopeList find_child_scopes() const
    {
        ChildScopeList list;
        list.push_back({"mock-scope-double-nav", true});
        list.push_back({"mock-scope", true});
        return list;
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
