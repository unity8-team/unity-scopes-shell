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

#pragma once

#include <scope-harness/results/result.h>

#include <vector>
#include <memory>

namespace unity
{
namespace scopeharness
{
namespace internal
{
struct CategoryArguments;
}
namespace results
{
class Result;

class Q_DECL_EXPORT Category final
{
public:
    typedef std::vector<Category> List;

    Category(Category&& other);

    Category(const Category& other);

    Category& operator=(const Category& other);

    Category& operator=(Category&& other);

    ~Category() = default;

    std::string id() const noexcept;

    std::string title() const noexcept;

    std::string icon() const noexcept;

    std::string headerLink() const noexcept;

    unity::scopes::Variant renderer() const;

    unity::scopes::Variant components() const;

    Result::List results() const;

    Result result(const std::string& uri) const;

    Result result(std::size_t index) const;

protected:
    friend view::ResultsView;

    Category(const internal::CategoryArguments& arguments);

    struct Priv;

    std::shared_ptr<Priv> p;
};

}
}
}
