/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *  Michał Sawicz <michal.sawicz@canonical.com>
 *  Michal Hruby <michal.hruby@canonical.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

// self
#include "previewstack.h"
#include "previewmodel.h"

// local
#include "utils.h"

// Qt
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace scopes_ng
{

using namespace unity;

PreviewStack::PreviewStack(QObject* parent) : QAbstractListModel(parent)
{
    m_previews.append(new PreviewModel(this));
}

PreviewStack::PreviewStack(PreviewModel* previewModel, QObject* parent) : QAbstractListModel(parent)
{
    if (previewModel != nullptr) {
        previewModel->setParent(this);
        m_previews.append(previewModel);
    } else {
        m_previews.append(new PreviewModel(this));
    }
}

QHash<int, QByteArray> PreviewStack::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[Roles::RolePreviewModel] = "previewModel";

    return roles;
}

void PreviewStack::setResult(std::shared_ptr<scopes::Result> const& result)
{
    m_previewedResult = result;
}

void PreviewStack::setWidgetColumnCount(int columnCount)
{
    if (m_widgetColumnCount != columnCount) {
        m_widgetColumnCount = columnCount;
        // set on all previews
        for (int i = 0; i < m_previews.size(); i++) {
            m_previews[i]->setWidgetColumnCount(columnCount);
        }
        Q_EMIT widgetColumnCountChanged();
    }
}

int PreviewStack::widgetColumnCount() const
{
    return m_widgetColumnCount;
}

int PreviewStack::rowCount(const QModelIndex&) const
{
    return m_previews.size();
}

PreviewModel* PreviewStack::get(int index) const
{
    if (index >= m_previews.size()) {
        return nullptr;
    }

    return m_previews.at(index);
}

QVariant PreviewStack::data(const QModelIndex& index, int role) const
{
    switch (role) {
        case RolePreviewModel:
            return QVariant::fromValue(m_previews.at(index.row()));
        default:
            return QVariant();
    }
}

} // namespace scopes_ng
