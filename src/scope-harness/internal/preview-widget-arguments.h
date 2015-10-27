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

#include <QtGlobal>
#include <QAbstractItemModel>

#include <memory>

namespace unity
{
namespace shell
{
namespace scopes
{
    class PreviewWidgetModelInterface;
    class PreviewModelInterface;
    class PreviewStackInterface;
}
}
namespace scopeharness
{
namespace view
{
class PreviewView;
class ResultsView;
}
namespace internal
{
struct PreviewWidgetArguments
{
    unity::shell::scopes::PreviewWidgetModelInterface* previewWidgetModel;

    QModelIndex index;

    unity::shell::scopes::PreviewModelInterface* previewModel;

    std::shared_ptr<view::ResultsView> resultsView;

    std::shared_ptr<view::PreviewView> previewView;

    std::shared_ptr<unity::shell::scopes::PreviewStackInterface> previewStack;
};
}
}
}
