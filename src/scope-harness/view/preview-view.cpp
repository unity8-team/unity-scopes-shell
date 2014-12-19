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
#include <scope-harness/view/preview-view.h>
#include <scope-harness/test-utils.h>

#include <unity/shell/scopes/PreviewModelInterface.h>
#include <unity/shell/scopes/PreviewStackInterface.h>
#include <unity/shell/scopes/PreviewWidgetModelInterface.h>


#include <QDebug>
#include <QSignalSpy>

using namespace std;
namespace ss = unity::shell::scopes;

namespace unity
{
namespace scopeharness
{
namespace view
{

struct PreviewView::Priv
{
    preview::PreviewWidget::List iterateWidgetModel(ss::PreviewWidgetModelInterface* previewWidgetModel)
    {
        preview::PreviewWidget::List previewWidgets;

        int rowCount = previewWidgetModel->rowCount();
        for (int row = 0; row < rowCount; ++row)
        {
            previewWidgets.emplace_back(
                    preview::PreviewWidget(internal::PreviewWidgetArguments
                    { previewWidgetModel, previewWidgetModel->index(row) }));
        }

        return previewWidgets;
    }

    vector<preview::PreviewWidget::List> iteratePreviewModel(ss::PreviewModelInterface* previewModel)
    {
        if (!previewModel->loaded())
        {
            QSignalSpy spy(previewModel, SIGNAL(loadedChanged()));
            spy.wait();
        }

        vector<preview::PreviewWidget::List> previewModels;

        int rowCount = previewModel->rowCount();
        for (int row = 0; row < rowCount; ++row)
        {
            QVariant var = previewModel->data(
                    previewModel->index(row),
                    ss::PreviewModelInterface::RoleColumnModel);

            previewModels.emplace_back(iterateWidgetModel(var.value<ss::PreviewWidgetModelInterface*>()));
        }

        return previewModels;
    }

    void setPreviewModel(shared_ptr<ss::PreviewStackInterface> previewStack)
    {
        m_previewStack = previewStack;
        updateModels();
    }

    void updateModels()
    {
        auto previewModel = m_previewStack->getPreviewModel(0);
        m_previewModels = iteratePreviewModel(previewModel);
    }

    void checkPreviewStack()
    {
        throwIfNot(bool(m_previewStack), "");
    }

    shared_ptr<ss::PreviewStackInterface> m_previewStack;

    vector<preview::PreviewWidget::List> m_previewModels;
};

PreviewView::PreviewView() :
        p(new Priv)
{
}

void PreviewView::preview(shared_ptr<ss::PreviewStackInterface> previewStack)
{
    p->setPreviewModel(previewStack);
}

void PreviewView::setColumnCount(unsigned int count)
{
    p->checkPreviewStack();

    p->m_previewStack->setWidgetColumnCount(count);
    // TODO Wait?
    p->updateModels();
}

unsigned int PreviewView::columnCount() const
{
    p->checkPreviewStack();

    return p->m_previewStack->widgetColumnCount();
}

vector<preview::PreviewWidget::List> PreviewView::widgets()
{
    p->checkPreviewStack();

    return p->m_previewModels;
}

preview::PreviewWidget::List PreviewView::widgetsInColumn(unsigned int column)
{
    p->checkPreviewStack();

    return p->m_previewModels.at(column);
}

preview::PreviewWidget::List PreviewView::widgetsInFirstColumn()
{
    return widgetsInColumn(0);
}

}
}
}
