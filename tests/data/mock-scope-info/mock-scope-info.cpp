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
 * Authored by: Marcus Tomlinson <marcus.tomlinson@canonical.com>
 */

#include <unity/scopes/ScopeBase.h>
#include <unity/scopes/SearchReply.h>

#include <atomic>

#define EXPORT __attribute__ ((visibility ("default")))

using namespace std;
using namespace unity::scopes;

class MyQuery : public SearchQueryBase
{
public:
    MyQuery(CannedQuery const& query, SearchMetadata const& metadata)
        : SearchQueryBase(query, metadata)
        , m_query(query.query_string())
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
        // No info (Status::Okay)
        if (m_query == "no_info")
        {
        }
        // NoInternet (Status::NoInternet)
        if (m_query == "no_internet")
        {
            reply->info(OperationInfo{OperationInfo::NoInternet});
        }
        // NoLocationData (Status::NoLocationData)
        if (m_query == "no_location")
        {
            reply->info(OperationInfo{OperationInfo::NoLocationData});
        }
        // DefaultSettingsUsed (unknown to shell but known to run-time so Status::Okay)
        if (m_query == "shell_unknown")
        {
            reply->info(OperationInfo{OperationInfo::DefaultSettingsUsed});
        }
        // DefaultSettingsUsed (unknown to runtime so Status::Unknown)
        if (m_query == "runtime_unknown")
        {
            reply->info(OperationInfo{static_cast<OperationInfo::InfoCode>(OperationInfo::LastInfoCode_ + 1)});
        }
        // NoLocationData and NoInternet (Status::NoInternet takes priority)
        if (m_query == "no_location_no_internet")
        {
            reply->info(OperationInfo{OperationInfo::NoLocationData});
            reply->info(OperationInfo{OperationInfo::NoInternet});
        }
    }

private:
    std::string m_query;
};

class MyScope : public ScopeBase
{
public:
    MyScope()
        : m_req_no(0)
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

private:
    atomic_int m_req_no;
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
