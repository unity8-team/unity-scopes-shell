/*
 * Copyright (C) 2015 Canonical, Ltd.
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

#include <scope-harness/view/filters-view.h>
#include <scope-harness/internal/filters-view-arguments.h>
#include <unity/shell/scopes/FiltersInterface.h>
#include <Unity/utils.h>
#include <scope-harness/test-utils.h>
#include <QSignalSpy>

namespace ss = unity::shell::scopes;
namespace sc = unity::scopes;

using namespace std;

namespace unity
{
namespace scopeharness
{
namespace view
{

struct FiltersView::_Priv
{
    ng::Scope::Ptr m_scope;
};

FiltersView::FiltersView(const internal::FiltersViewArguments& args)
    : p(new _Priv)
{
    p->m_scope = args.m_scope;
}

std::size_t FiltersView::size() const
{
    return p->m_scope->filters()->rowCount();
}

}
}
}
