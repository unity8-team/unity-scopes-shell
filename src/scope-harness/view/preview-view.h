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

#include <scope-harness/view/abstract-view.h>
#include <scope-harness/preview/preview-widget-list.h>

namespace unity
{
namespace shell
{
namespace scopes
{
class PreviewStackInterface;
}
}
namespace scopeharness
{
class ScopeHarness;

namespace results
{
class Result;
}
namespace view
{
class ResultsView;

class Q_DECL_EXPORT PreviewView final: public AbstractView
{
public:
    UNITY_DEFINES_PTRS(PreviewView);

    PreviewView();

    ~PreviewView() = default;

    PreviewView(const PreviewView& other) = delete;

    PreviewView(PreviewView&& other) = delete;

    PreviewView& operator=(const PreviewView& other) = delete;

    PreviewView& operator=(PreviewView&& other) = delete;

    void setColumnCount(unsigned int count);

    unsigned int columnCount() const;

    std::vector<preview::PreviewWidgetList> widgets();

    preview::PreviewWidgetList widgetsInColumn(std::size_t column);

    preview::PreviewWidgetList widgetsInFirstColumn();

protected:
    friend results::Result;
    friend ScopeHarness;
    friend preview::PreviewWidget;

    void preview(std::shared_ptr<shell::scopes::PreviewStackInterface> previewStack);

    void setResultsView(std::shared_ptr<ResultsView> resultsView);

    void refresh();

    struct _Priv;

    std::shared_ptr<_Priv> p;
};

}
}
}
