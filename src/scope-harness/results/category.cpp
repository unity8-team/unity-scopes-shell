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

#include <scope-harness/results/category.h>
#include <scope-harness/internal/category-arguments.h>
#include <scope-harness/test-utils.h>

#include <unity/shell/scopes/CategoriesInterface.h>

#include <Unity/utils.h>

#include <boost/regex.hpp>

using namespace std;
using namespace boost;
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

struct Category::_Priv
{
    ss::CategoriesInterface* m_categoriesModel;
    QModelIndex m_index;
    Result::List m_results;
};

Category::Category(const internal::CategoryArguments& arguments) :
        p(new _Priv)
{
    p->m_categoriesModel = arguments.categoriesModel;
    p->m_index = arguments.index;
    p->m_results = arguments.results;
}

Category::Category(Category&& other) :
        p(new _Priv)
{
    *this = std::move(other);
}

Category::Category(const Category& other) :
        p(new _Priv)
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
    p = std::move(other.p);
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

Result::List Category::results() const
{
    return p->m_results;
}

size_t Category::size() const
{
    return p->m_results.size();
}

bool Category::empty() const
{
    return p->m_results.empty();
}

Result Category::result(const string& uri) const
{
    regex e(uri);
    for (const auto& result : p->m_results)
    {
        if (regex_match(result.uri(), e))
        {
            return result;
        }
    }

    throw domain_error("Result with URI '" + uri + "' could not be found");
}

Result Category::result(size_t index) const
{
    TestUtils::throwIf(index >= p->m_results.size(), "Invalid index " + to_string(index) + " in result lookup");

    return p->m_results.at(index);
}

}
}
}
