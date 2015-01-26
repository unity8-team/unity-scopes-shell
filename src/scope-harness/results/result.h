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

#include <scope-harness/view/abstract-view.h>

#include <unity/scopes/Variant.h>

#include <QtGlobal>

#include <vector>

#pragma once

namespace unity
{
namespace scopeharness
{
namespace internal
{
struct ResultArguments;
}
namespace view
{
class ResultsView;
class PreviewView;
}
namespace results
{

class Q_DECL_EXPORT Result final
{
public:
    typedef std::vector<Result> List;

    Result(Result&& other);

    Result(const Result& other);

    Result& operator=(const Result& other);

    Result& operator=(Result&& other);

    ~Result() = default;

    unity::scopes::Variant const& operator[](std::string const& key) const;

    std::string uri() const noexcept;

    std::string title() const noexcept;

    std::string art() const noexcept;

    std::string dnd_uri() const noexcept;

    std::string subtitle() const noexcept;

    std::string emblem() const noexcept;

    std::string mascot() const noexcept;

    unity::scopes::Variant attributes() const noexcept;

    unity::scopes::Variant summary() const noexcept;

    unity::scopes::Variant background() const noexcept;

    unity::scopes::Variant const& value(std::string const& key) const;

    view::AbstractView::SPtr activate() const;

protected:
    friend view::ResultsView;
    friend view::PreviewView;

    Result(const internal::ResultArguments& arguments);

    struct Priv;

    std::shared_ptr<Priv> p;
};

}
}
}
