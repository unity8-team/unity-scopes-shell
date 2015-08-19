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

#include <scope-harness/internal/preview-widget-arguments.h>
#include <scope-harness/preview/preview-widget.h>
#include <scope-harness/view/abstract-view.h>
#include <scope-harness/view/preview-view.h>
#include <scope-harness/view/results-view.h>
#include <scope-harness/test-utils.h>

#include <unity/shell/scopes/PreviewModelInterface.h>
#include <unity/shell/scopes/PreviewWidgetModelInterface.h>

#include <Unity/utils.h>
#include <Unity/previewstack.h>

#include <QDebug>
#include <QSignalSpy>
#include <memory>

using namespace std;
namespace ng = scopes_ng;
namespace sc = unity::scopes;
namespace ss = unity::shell::scopes;

namespace unity
{
namespace scopeharness
{
using namespace internal;
namespace preview
{

struct PreviewWidget::_Priv
{
    unity::shell::scopes::PreviewWidgetModelInterface* m_previewWidgetModel;

    QModelIndex m_index;

    unity::shell::scopes::PreviewModelInterface* m_previewModel;

    weak_ptr<view::ResultsView> m_resultsView;

    weak_ptr<view::PreviewView> m_previewView;

    std::shared_ptr<unity::shell::scopes::PreviewStackInterface> m_previewStack;
};

PreviewWidget::PreviewWidget(const internal::PreviewWidgetArguments& arguments) :
        p(new _Priv)
{
    p->m_previewWidgetModel = arguments.previewWidgetModel;
    p->m_previewModel = arguments.previewModel;
    p->m_index = arguments.index;
    p->m_resultsView = arguments.resultsView;
    p->m_previewView = arguments.previewView;
    p->m_previewStack = arguments.previewStack;
}


PreviewWidget::PreviewWidget(const PreviewWidget& other) :
        p(new _Priv)
{
    *this = other;
}

PreviewWidget::PreviewWidget(PreviewWidget&& other)
{
    *this = std::move(other);
}

PreviewWidget& PreviewWidget::operator=(const PreviewWidget& other)
{
    p->m_previewWidgetModel = other.p->m_previewWidgetModel;
    p->m_previewModel = other.p->m_previewModel;
    p->m_index = other.p->m_index;
    p->m_resultsView = other.p->m_resultsView;
    p->m_previewView = other.p->m_previewView;
    p->m_previewStack = other.p->m_previewStack;
    return *this;
}

PreviewWidget& PreviewWidget::operator=(PreviewWidget&& other)
{
    p = std::move(other.p);
    return *this;
}

PreviewWidget::~PreviewWidget()
{
}

std::string PreviewWidget::id() const
{
    return p->m_previewWidgetModel->data(
            p->m_index, ss::PreviewWidgetModelInterface::RoleWidgetId).toString().toStdString();
}

std::string PreviewWidget::type() const
{
    return p->m_previewWidgetModel->data(
            p->m_index, ss::PreviewWidgetModelInterface::RoleType).toString().toStdString();
}

sc::Variant PreviewWidget::data() const
{
    return ng::qVariantToScopeVariant(
            p->m_previewWidgetModel->data(
                    p->m_index,
                    ss::PreviewWidgetModelInterface::RoleProperties));
}

view::AbstractView::SPtr PreviewWidget::trigger(const string& name, const sc::Variant& v)
{
    auto ps = std::dynamic_pointer_cast<ng::PreviewStack>(p->m_previewStack);
    TestUtils::throwIfNot(bool(ps), "No preview stack");
    TestUtils::throwIfNot(bool(ps->associatedScope()), "Preview stack has no associated scope");
    QSignalSpy showDashSpy(ps->associatedScope(), SIGNAL(showDash()));

    QVariant widgetData;

    if (type() == "actions" &&
            v.which() == sc::Variant::Dict &&
            v.get_dict()["actions"].which() == sc::Variant::Array)
    {
        for (auto el: v.get_dict()["actions"].get_array())
        {
            auto d = el.get_dict();
            if (d["id"].get_string() == name)
            {
                widgetData = ng::scopeVariantToQVariant(el);
                break;
            }
        }
    }
    else
    {
        widgetData = ng::scopeVariantToQVariant(v);
    }

    Q_EMIT p->m_previewModel->triggered(
            QString::fromStdString(id()), QString::fromStdString(name),
            widgetData.toMap());

    if (!showDashSpy.empty())
    {
        return p->m_resultsView.lock();
    }

    TestUtils::throwIfNot(p->m_previewModel->processingAction(), "Should be processing action");
    QSignalSpy spy(p->m_previewModel, SIGNAL(processingActionChanged()));
    TestUtils::throwIfNot(spy.wait(), "Processing action property didn't change");
    TestUtils::throwIf(p->m_previewModel->processingAction(), "Should have finished processing action");

    view::PreviewView::SPtr previewView = p->m_previewView.lock();
    previewView->refresh();

    return p->m_previewView.lock();
}

}
}
}
