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
#include <QSharedPointer>

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
    class ResultsModelInterface;
    class ScopeInterface;
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
struct ResultArguments
{
    QSharedPointer<unity::shell::scopes::ResultsModelInterface> resultsModel;

    QSharedPointer<unity::shell::scopes::ScopeInterface> scope;

    const QModelIndex& index;

    std::shared_ptr<view::ResultsView> resultsView;

    std::shared_ptr<view::PreviewView> previewView;
};
}
}
}
