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

#include <internal/preview-widget-list-arguments.h>
#include <preview/preview-widget-list.h>

using namespace std;

namespace unity
{
namespace scopeharness
{
namespace preview
{

struct PreviewWidgetList::Priv
{
    std::vector<preview::PreviewWidget> previewWidgets;
};

PreviewWidgetList::PreviewWidgetList(const internal::PreviewWidgetListArguments& arguments) :
    p(new Priv)
{
    p->previewWidgets = arguments.previewWidgets;
}


PreviewWidgetList::PreviewWidgetList(const PreviewWidgetList& other) :
        p(new Priv)
{
    *this = other;
}

PreviewWidgetList::PreviewWidgetList(PreviewWidgetList&& other)
{
    *this = move(other);
}

PreviewWidgetList::~PreviewWidgetList()
{
}

PreviewWidgetList& PreviewWidgetList::operator=(const PreviewWidgetList& other)
{
    p->previewWidgets = other.p->previewWidgets;
    return *this;
}

PreviewWidgetList& PreviewWidgetList::operator=(PreviewWidgetList&& other)
{
    p = move(other.p);
    return *this;
}

PreviewWidget PreviewWidgetList::at(std::size_t index) const
{
    return p->previewWidgets.at(index);
}

PreviewWidget PreviewWidgetList::at(const std::string& id) const
{
    for (const auto& widget : p->previewWidgets)
    {
        if (widget.id() == id)
        {
            return widget;
        }
    }

    throw domain_error("Widget '" + id + "' not found");
}

PreviewWidget PreviewWidgetList::operator[](std::size_t index) const
{
    return at(index);
}

PreviewWidget PreviewWidgetList::operator[](const std::string& id) const
{
    return at(id);
}

std::size_t PreviewWidgetList::size() const
{
    return p->previewWidgets.size();
}

}
}
}
