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

#include <scope-harness/category.h>

#include <unity/shell/scopes/CategoriesInterface.h>

#include <Unity/utils.h>

using namespace std;
namespace ng = scopes_ng;
namespace sc = unity::scopes;
namespace ss = unity::shell::scopes;

namespace unity
{
namespace scopeharness
{

struct Category::Priv
{
    ss::CategoriesInterface* m_categoriesModel;
    QModelIndex m_index;
    Result::List m_results;
};

Category::Category(ss::CategoriesInterface* categoriesModel,
                   const QModelIndex& index, const Result::List& results) :
        p(new Priv)
{
    p->m_categoriesModel = categoriesModel;
    p->m_index = index;
    p->m_results = results;
}

Category::Category(const Category& other) :
        p(new Priv)
{
    *this = other;
}

Category& Category::operator=(const Category& other)
{
    p->m_categoriesModel = other.p->m_categoriesModel;
    p->m_index = other.p->m_index;
    p->m_results = other.p->m_results;
    return *this;
}

Category& Category::operator=(Category&& other)
{
    p = move(other.p);
    return *this;
}

string Category::id() const noexcept
{
    return p->m_categoriesModel->data(p->m_index,
                                   ss::CategoriesInterface::Roles::RoleCategoryId).toString().toStdString();
}

string Category::title() const noexcept
{
    return p->m_categoriesModel->data(p->m_index,
                                       ss::CategoriesInterface::Roles::RoleName).toString().toStdString();
}

string Category::icon() const noexcept
{
    return p->m_categoriesModel->data(p->m_index,
                                       ss::CategoriesInterface::Roles::RoleIcon).toString().toStdString();
}

string Category::headerLink() const noexcept
{
    return p->m_categoriesModel->data(p->m_index,
                                       ss::CategoriesInterface::Roles::RoleHeaderLink).toString().toStdString();
}

sc::Variant Category::renderer() const
{
    return ng::qVariantToScopeVariant(p->m_categoriesModel->data(p->m_index,
                                       ss::CategoriesInterface::Roles::RoleRenderer));
}

sc::Variant Category::components() const
{
    return ng::qVariantToScopeVariant(p->m_categoriesModel->data(p->m_index,
                                       ss::CategoriesInterface::Roles::RoleComponents));
}

const Result::List& Category::results() const
{
    return p->m_results;
}

}
}
