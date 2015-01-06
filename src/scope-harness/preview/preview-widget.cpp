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

#include <unity/shell/scopes/PreviewModelInterface.h>
#include <unity/shell/scopes/PreviewWidgetModelInterface.h>

#include <Unity/utils.h>

#include <QDebug>

using namespace std;
namespace ng = scopes_ng;
namespace sc = unity::scopes;
namespace ss = unity::shell::scopes;

namespace unity
{
namespace scopeharness
{
namespace preview
{

struct PreviewWidget::Priv
{
    internal::PreviewWidgetArguments m_arguments;
};

PreviewWidget::PreviewWidget(const internal::PreviewWidgetArguments& arguments) :
        p(new Priv)
{
    p->m_arguments = arguments;
}


PreviewWidget::PreviewWidget(const PreviewWidget& other) :
        p(new Priv)
{
    *this = other;
}

PreviewWidget::PreviewWidget(PreviewWidget&& other)
{
    *this = move(other);
}

PreviewWidget& PreviewWidget::operator=(const PreviewWidget& other)
{
    p->m_arguments = other.p->m_arguments;
    return *this;
}

PreviewWidget& PreviewWidget::operator=(PreviewWidget&& other)
{
    p = move(other.p);
    return *this;
}

PreviewWidget::~PreviewWidget()
{
}

std::string PreviewWidget::id() const
{
    return p->m_arguments.previewWidgetModel->data(
            p->m_arguments.index, ss::PreviewWidgetModelInterface::RoleWidgetId).toString().toStdString();
}

std::string PreviewWidget::type() const
{
    return p->m_arguments.previewWidgetModel->data(
            p->m_arguments.index, ss::PreviewWidgetModelInterface::RoleType).toString().toStdString();
}

sc::Variant PreviewWidget::data() const
{
    return ng::qVariantToScopeVariant(
            p->m_arguments.previewWidgetModel->data(
                    p->m_arguments.index,
                    ss::PreviewWidgetModelInterface::RoleProperties));
}

void PreviewWidget::trigger(const string& name, const sc::Variant& v)
{
    Q_EMIT p->m_arguments.previewModel->triggered(
            QString::fromStdString(id()), QString::fromStdString(name),
            ng::scopeVariantToQVariant(v).toMap());
}

}
}
}
