/*
 * match-result.cpp
 *
 *  Created on: 2 Dec 2014
 *      Author: pete
 */

#include <scope-harness/match-result.h>

#include <sstream>

using namespace std;

namespace unity
{
namespace scopeharness
{

struct MatchResult::Priv
{
    bool m_success = true;

    deque<string> m_failures;
};

MatchResult::MatchResult() :
        p(new Priv)
{
}

MatchResult::MatchResult(const MatchResult& other) :
        p(new Priv)
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
    p = move(other.p);
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

deque<string>& MatchResult::failures() const
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
