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

#include <scope-harness/matcher/match-result.h>

#include <sstream>

using namespace std;

namespace unity
{
namespace scopeharness
{
namespace matcher
{

struct MatchResult::_Priv
{
    bool m_success = true;

    vector<string> m_failures;
};

MatchResult::MatchResult() :
        p(new _Priv)
{
}

MatchResult::MatchResult(MatchResult&& other)
{
    *this = std::move(other);
}

MatchResult::MatchResult(const MatchResult& other) :
        p(new _Priv)
{
    *this = other;
}

MatchResult& MatchResult::operator=(const MatchResult& other)
{
    p->m_success = other.p->m_success;
    p->m_failures= other.p->m_failures;
    return *this;
}

MatchResult& MatchResult::operator=(MatchResult&& other)
{
    p = std::move(other.p);
    return *this;
}

void MatchResult::failure(const string& message)
{
    p->m_success = false;
    p->m_failures.emplace_back(message);
}

bool MatchResult::success() const
{
    return p->m_success;
}

vector<string>& MatchResult::failures() const
{
    return p->m_failures;
}

string MatchResult::concat_failures() const
{
    stringstream ss;
    ss << "Failed expectations:" << endl;
    for (const auto& failure : p->m_failures)
    {
        ss << failure << endl;
    }
    return ss.str();
}

}
}
}
