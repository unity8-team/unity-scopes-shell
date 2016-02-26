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
#include <scope-harness/internal/preview-widget-list-arguments.h>
#include <scope-harness/view/preview-view.h>
#include <scope-harness/view/results-view.h>
#include <scope-harness/test-utils.h>

#include <unity/shell/scopes/PreviewModelInterface.h>
#include <unity/shell/scopes/PreviewWidgetModelInterface.h>


#include <QDebug>
#include <QSignalSpy>

using namespace std;
namespace ss = unity::shell::scopes;

namespace unity
{
namespace scopeharness
{
using namespace internal;
namespace view
{

struct PreviewView::_Priv
{
    preview::PreviewWidgetList iterateWidgetModel(ss::PreviewWidgetModelInterface* previewWidgetModel,
                                                    ss::PreviewModelInterface* previewModel, PreviewView::SPtr previewView)
    {
        vector<preview::PreviewWidget> previewWidgets;

        int rowCount = previewWidgetModel->rowCount();
        for (int row = 0; row < rowCount; ++row)
        {
            previewWidgets.emplace_back(
                    preview::PreviewWidget(internal::PreviewWidgetArguments
                    { previewWidgetModel, previewWidgetModel->index(row), previewModel, m_resultsView.lock(), previewView }));
        }

        return preview::PreviewWidgetList(internal::PreviewWidgetListArguments{previewWidgets});
    }

    vector<preview::PreviewWidgetList> iteratePreviewModel(ss::PreviewModelInterface* previewModel, PreviewView::SPtr previewView)
    {
        if (!previewModel->loaded())
        {
            QSignalSpy spy(previewModel, SIGNAL(loadedChanged()));
            spy.wait();
        }

        vector<preview::PreviewWidgetList> previewModels;

        int rowCount = previewModel->rowCount();
        for (int row = 0; row < rowCount; ++row)
        {
            QVariant var = previewModel->data(
                    previewModel->index(row),
                    ss::PreviewModelInterface::RoleColumnModel);

            previewModels.emplace_back(iterateWidgetModel(var.value<ss::PreviewWidgetModelInterface*>(), previewModel, previewView));
        }

        return previewModels;
    }

    void setPreviewModel(shared_ptr<ss::PreviewModelInterface> previewModel, PreviewView::SPtr previewView)
    {
        m_previewModel = previewModel;
        updateModels(previewView);
    }

    void updateModels(PreviewView::SPtr previewView)
    {
        m_previewModels = iteratePreviewModel(m_previewModel.get(), previewView);
    }

    void checkPreviewModel()
    {
        TestUtils::throwIfNot(bool(m_previewModel), "");
    }

    shared_ptr<ss::PreviewModelInterface> m_previewModel;

    vector<preview::PreviewWidgetList> m_previewModels;

    weak_ptr<ResultsView> m_resultsView;
};

PreviewView::PreviewView() :
        p(new _Priv)
{
}

void PreviewView::setResultsView(ResultsView::SPtr resultsView)
{
    p->m_resultsView = resultsView;
}

void PreviewView::preview(shared_ptr<ss::PreviewModelInterface> previewModel)
{
    p->setPreviewModel(previewModel,
                       dynamic_pointer_cast<PreviewView>(shared_from_this()));
}

void PreviewView::setColumnCount(unsigned int count)
{
    p->checkPreviewModel();

    p->m_previewModel->setWidgetColumnCount(count);
    // TODO Wait?
    refresh();
}

void PreviewView::refresh()
{
    p->updateModels(dynamic_pointer_cast<PreviewView>(shared_from_this()));
}

unsigned int PreviewView::columnCount() const
{
    p->checkPreviewModel();

    return p->m_previewModel->widgetColumnCount();
}

vector<preview::PreviewWidgetList> PreviewView::widgets()
{
    p->checkPreviewModel();

    return p->m_previewModels;
}

preview::PreviewWidgetList PreviewView::widgetsInColumn(size_t column)
{
    p->checkPreviewModel();

    return p->m_previewModels.at(column);
}

preview::PreviewWidgetList PreviewView::widgetsInFirstColumn()
{
    return widgetsInColumn(0);
}

}
}
}
