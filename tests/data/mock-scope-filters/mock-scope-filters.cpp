/*
 * Copyright (C) 2015 Canonical Ltd
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
 * Authored by: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <unity/scopes/CategorisedResult.h>
#include <unity/scopes/OptionSelectorFilter.h>
#include <unity/scopes/RangeInputFilter.h>
#include <unity/scopes/ValueSliderFilter.h>
#include <unity/scopes/ValueSliderLabels.h>
#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/SearchReply.h>

#include <sstream>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::scopes;

class MyQuery : public SearchQueryBase
{
public:
    MyQuery(CannedQuery const& query, SearchMetadata const& metadata) :
        SearchQueryBase(query, metadata)
    {
    }

    ~MyQuery()
    {
    }

    virtual void cancelled() override
    {
    }

    virtual void run(SearchReplyProxy const& reply) override
    {
        OptionSelectorFilter::UPtr filter1 = OptionSelectorFilter::create("f1", "Filter1");
        auto opt1 = filter1->add_option("o1", "Option1");
        filter1->add_option("o2", "Option2");

        RangeInputFilter::SPtr filter2 = RangeInputFilter::create("f2", Variant(2.0f), Variant::null(), "start", "", "", "end", "");

        auto cat1 = reply->register_category("cat1", "Category 1", "");
        CategorisedResult res1(cat1);
        res1.set_uri("test:uri");

        auto selected = filter1->active_options(query().filter_state());
        if (selected.size() == 1) {
            res1.set_title("result for option " + (*selected.begin())->id());
        } else {
            res1.set_title("result for: \"" + query().query_string() + "\"");
        }

        bool has_start_val = filter2->has_start_value(query().filter_state());
        bool has_end_val = filter2->has_end_value(query().filter_state());

        if (has_start_val || has_end_val)
        {
            res1.set_title("result for range: " +
                    (has_start_val ? std::to_string(filter2->start_value(query().filter_state())) : "***") + " - " +
                    (has_end_val ? std::to_string(filter2->end_value(query().filter_state())) : "***"));
        }

        ValueSliderFilter::SPtr filter3 = ValueSliderFilter::create("f3", 1, 99, 50, ValueSliderLabels("Min", "Max", {{33, "One third"}}));

        Filters filters;
        filters.push_back(std::move(filter1));
        filters.push_back(std::move(filter2));
        filters.push_back(std::move(filter3));

        reply->push(filters, query().filter_state());
        reply->push(res1);
    }
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
