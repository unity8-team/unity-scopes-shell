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

#include <scope-harness/preview/preview-widget.h>

namespace unity
{
namespace scopeharness
{
namespace internal
{
struct PreviewWidgetListArguments;
}
namespace view
{
class PreviewView;
}
namespace preview
{

class Q_DECL_EXPORT PreviewWidgetList
{
public:
    PreviewWidgetList(const PreviewWidgetList& other);

    PreviewWidgetList(PreviewWidgetList&& other);

    PreviewWidgetList& operator=(const PreviewWidgetList& other);

    PreviewWidgetList& operator=(PreviewWidgetList&& other);

    ~PreviewWidgetList();

    PreviewWidget at(std::size_t index) const;

    PreviewWidget at(const std::string& id) const;

    PreviewWidget operator[](std::size_t index) const;

    PreviewWidget operator[](const std::string& id) const;

    std::size_t size() const;

protected:
    friend view::PreviewView;

    PreviewWidgetList(const internal::PreviewWidgetListArguments& arguments);

    struct _Priv;

    std::shared_ptr<_Priv> p;
};

}
}
}
