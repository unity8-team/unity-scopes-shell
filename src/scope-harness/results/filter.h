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

#pragma once

namespace unity
{
namespace scopeharness
{
namespace internal
{
//struct ResultArguments; FIXME: remove
}
namespace view
{
class ResultsView;
}
namespace results
{

class Q_DECL_EXPORT Filter final
{
public:
    typedef std::vector<Filter> List;

    Filter(Filter&& other);

    Filter(const Filter& other);

    Filter& operator=(const Filter& other);

    Filter& operator=(Filter&& other);

    ~Filter() = default;

    std::string id() const noexcept;

    std::string title() const noexcept;

protected:
    Filter();
    friend view::ResultsView;

    struct _Priv;

    std::shared_ptr<_Priv> p;
};

}
}
}
