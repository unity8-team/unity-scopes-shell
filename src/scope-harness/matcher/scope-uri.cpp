/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of version 3 of the GNU Lesser General Public License as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Author: Pete Woods <pete.woods@canonical.com>
 */

#include <scope-harness/matcher/scope-uri.h>

#include <QUrl>

#include <sstream>

using namespace std;

namespace unity
{
namespace scopeharness
{
namespace matcher
{
namespace
{
static void paramJoin(stringstream& s, const vector<pair<string, string>>& v)
{
    bool first = true;
    for (const auto& e : v)
    {
        if (first)
        {
            first = false;
            s << "\\?";
        }
        else
        {
            s << "&";
        }
        s
                << QUrl::toPercentEncoding(QString::fromStdString(e.first)).constData()
                << "="
                << QUrl::toPercentEncoding(QString::fromStdString(e.second)).constData();
    }
}
}

struct ScopeUri::Priv
{
    string m_id;

    string m_department;

    string m_query;
};

ScopeUri::ScopeUri(const string& id) :
        p(new Priv)
{
    p->m_id = id;
}

ScopeUri::~ScopeUri()
{
}

ScopeUri::ScopeUri(const ScopeUri& other) :
        p(new Priv)
{
    *this = other;
}

ScopeUri::ScopeUri(ScopeUri&& other)
{
    *this = move(other);
}

ScopeUri& ScopeUri::operator=(const ScopeUri& other)
{
    p->m_id = other.p->m_id;
    p->m_department = other.p->m_department;
    p->m_query = other.p->m_query;
    return *this;
}

ScopeUri& ScopeUri::operator=(ScopeUri&& other)
{
    p = move(other.p);
    return *this;
}

ScopeUri& ScopeUri::department(const std::string& departmentId)
{
    p->m_department = departmentId;
    return *this;
}

ScopeUri& ScopeUri::query(const std::string& queryString)
{
    p->m_query = queryString;
    return *this;
}

string ScopeUri::toString() const
{
    stringstream result;
    result << "scope:\\/\\/"
            << QUrl::toPercentEncoding(QString::fromStdString(p->m_id)).constData();
    vector<pair<string, string>> params;
    if (!p->m_department.empty())
    {
        params.emplace_back(make_pair("department", p->m_department));
    }
    if (!p->m_query.empty())
    {
        params.emplace_back(make_pair("q", p->m_query));
    }
    paramJoin(result, params);
    return result.str();
}

}
}
}
