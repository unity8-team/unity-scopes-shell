/*
 * Copyright (C) 2016 Canonical, Ltd.
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
 * Author: Pawel Stolowski <pawel.stolowski@canonical.com>
 */

#include <scope-harness/test-utils.h>
#include <scope-harness/results/filter.h>
#include <scope-harness/view/filters-view.h>
#include <scope-harness/internal/filter-arguments.h>

#include <Unity/utils.h>
#include <Unity/filters.h>

#include <QObject>
#include <QSignalSpy>

using namespace std;

namespace ng = scopes_ng;
namespace sc = unity::scopes;
namespace ss = unity::shell::scopes;

namespace unity
{
namespace scopeharness
{
using namespace internal;

namespace results
{

struct Filter::_Priv: public QObject
{
public:
    QSharedPointer<ss::FiltersInterface> m_FiltersModel;

    QSharedPointer<ss::ScopeInterface> m_scope;

    weak_ptr<view::FiltersView> m_FiltersView;
};

Filter::Filter(internal::FilterArguments const& args) :
        p(new _Priv)
{
    //TODO
}

Filter::Filter(Filter&& other)
{
    *this = std::move(other);
}

Filter::Filter(const Filter& other) :
        p(new _Priv)
{
    *this = other;
}

Filter& Filter::operator=(const Filter& other)
{
    // TODO
    return *this;
}

Filter& Filter::operator=(Filter&& other)
{
    p = std::move(other.p);
    return *this;
}

string Filter::id() const noexcept
{
    return ""; //TODO
}

string Filter::title() const noexcept
{
    return ""; //TODO
}

}
}
}
