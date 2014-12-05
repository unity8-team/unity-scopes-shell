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

#include <scope-harness/result.h>

#include <qglobal.h>

#include <deque>
#include <memory>

QT_BEGIN_NAMESPACE
class QModelIndex;
QT_END_NAMESPACE

namespace unity
{
namespace shell
{
namespace scopes
{
    class CategoriesInterface;
}
}
namespace scopeharness
{

class Result;

class Q_DECL_EXPORT Category
{
public:
    typedef std::deque<Category> List;

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

    const Result::List& results() const;

protected:
    friend ResultsView;

    Category(unity::shell::scopes::CategoriesInterface* categoriesModel,
             const QModelIndex& index, const Result::List& results);

    struct Priv;

    std::shared_ptr<Priv> p;
};

}
}
