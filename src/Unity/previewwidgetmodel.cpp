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
#include "previewwidgetmodel.h"

// local
#include "utils.h"

// Qt
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace scopes_ng
{

using namespace unity;

PreviewWidgetModel::PreviewWidgetModel(QObject* parent)
 : unity::shell::scopes::PreviewWidgetModelInterface(parent)
{
}

void PreviewWidgetModel::insertWidget(QSharedPointer<PreviewWidgetData> const& widget, int position)
{
    int insertPos = position >= 0 && position <= m_previewWidgets.count() ? position : m_previewWidgets.count();
#ifdef VERBOSE_MODEL_UPDATES
    qDebug() << "PreviewWidgetModel::insertWidget(): inserting widget" << widget->id << "at" << insertPos;
#endif

    beginInsertRows(QModelIndex(), insertPos, insertPos);

    m_previewWidgets.insert(insertPos, widget);

    endInsertRows();
}

void PreviewWidgetModel::addWidgets(QList<QSharedPointer<PreviewWidgetData>> const& widgetList)
{
    if (widgetList.size() == 0) return;

    beginInsertRows(QModelIndex(), m_previewWidgets.count(), m_previewWidgets.size() + widgetList.size() - 1);

    m_previewWidgets.append(widgetList);

    endInsertRows();
}

void PreviewWidgetModel::updateWidget(QSharedPointer<PreviewWidgetData> const& widget, int row)
{
    auto oldWidget = m_previewWidgets.at(row);
    if (oldWidget->id != widget->id) {
        qWarning() << "PreviewWidgetModel::updateWidget(): unexpected widget" << widget->id;
        return;
    }

#ifdef VERBOSE_MODEL_UPDATES
    qDebug() << "PreviewWidgetModel::updateWidget(): updating widget" << widget->id << " at row" << row << ",data=" << widget->data;
#endif
    m_previewWidgets.replace(row, widget);
    const QModelIndex idx = createIndex(row, 0);
    Q_EMIT dataChanged(idx, idx);
}

void PreviewWidgetModel::updateWidget(QSharedPointer<PreviewWidgetData> const& updatedWidget)
{
    for (int i = 0; i<m_previewWidgets.count(); i++) {
        auto widget = m_previewWidgets.at(i);
        if (updatedWidget->id == widget->id) {
#ifdef VERBOSE_MODEL_UPDATES
            qDebug() << "PreviewWidgetModel::updateWidget(): updating widget" << widget->id << " at row" << i << ",data=" << widget->data;
#endif
            m_previewWidgets.replace(i, updatedWidget);
            const QModelIndex idx = createIndex(i, 0);
            Q_EMIT dataChanged(idx, idx);
            break;
        }
    }
}

void PreviewWidgetModel::clearWidgets()
{
    beginRemoveRows(QModelIndex(), 0, m_previewWidgets.count() - 1);
    m_previewWidgets.clear();
    endRemoveRows();
}

bool PreviewWidgetModel::widgetChanged(PreviewWidgetData* widget)
{
    for (int i = 0; i < m_previewWidgets.size(); i++) {
        if (m_previewWidgets[i].data() == widget) {
            QModelIndex changedIndex(index(i));
            QVector<int> changedRoles;
            changedRoles.append(PreviewWidgetModel::RoleProperties);
            dataChanged(changedIndex, changedIndex, changedRoles);

            return true;
        }
    }

    return false;
}


void PreviewWidgetModel::removeWidget(QSharedPointer<PreviewWidgetData> const& widget)
{
    int index = widgetIndex(widget);
    if (index >= 0) {
#ifdef VERBOSE_MODEL_UPDATES
        qDebug() << "PreviewWidgetModel::removeWidget(): removing widget" << widget->id << "at row" << index;
#endif
        beginRemoveRows(QModelIndex(), index, index);
        m_previewWidgets.removeAt(index);
        endRemoveRows();
    } else {
        qWarning() << "PreviewWidgetModel::removeWidget(): widget" << widget->id << "doesn't exist in the column model";
    }
}

int PreviewWidgetModel::widgetIndex(QString const &widgetId) const
{
    // TODO optimize
    for (int i = 0; i<m_previewWidgets.size(); i++) {
        if (m_previewWidgets.at(i)->id == widgetId) {
            return i;
        }
    }
    return -1;
}

int PreviewWidgetModel::widgetIndex(QSharedPointer<PreviewWidgetData> const& widget) const
{
    // TODO optimize
    for (int i = 0; i<m_previewWidgets.size(); i++) {
        if (*(m_previewWidgets.at(i)) == *widget) {
            return i;
        }
    }
    return -1;
}

void PreviewWidgetModel::moveWidget(QSharedPointer<PreviewWidgetData> const& widget, int sourceRow, int destRow)
{
    if (destRow < 0 || destRow >= m_previewWidgets.size()) {
        qWarning() << "PreviewWidgetModel::moveWidget(): invalid destRow" << destRow;
        return;
    }
    if (sourceRow < 0 || sourceRow >= m_previewWidgets.size()) {
        qWarning() << "PreviewWidgetModel::moveWidget(): invalid sourceRow" << sourceRow;
        return;
    }

    if (m_previewWidgets.at(sourceRow)->id != widget->id) {
        qWarning() << "PreviewWidgetModel::moveWidget(): unexpected widget" << widget->id;
        return;
    }
#ifdef VERBOSE_MODEL_UPDATES
    qDebug() << "Moving widget" << widget->id << "from" << sourceRow << "to" << destRow;
#endif
    beginMoveRows(QModelIndex(), sourceRow, sourceRow, QModelIndex(), destRow + (destRow > sourceRow ? 1 : 0));
    m_previewWidgets.move(sourceRow, destRow);
    endMoveRows();
}

int PreviewWidgetModel::rowCount(const QModelIndex&) const
{
    return m_previewWidgets.size();
}

QSharedPointer<PreviewWidgetData> PreviewWidgetModel::widget(int index) const
{
    return m_previewWidgets.at(index);
}

QVariant PreviewWidgetModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if (row >= m_previewWidgets.size())
    {
        qWarning() << "PreviewWidgetModel::data - invalid index" << row << "size"
                << m_previewWidgets.size();
        return QVariant();
    }

    auto widget_data = m_previewWidgets.at(index.row());
    switch (role) {
        case RoleWidgetId:
            return widget_data->id;
        case RoleType:
            return widget_data->type;
        case RoleProperties:
            return widget_data->data;
        default:
            return QVariant();
    }
}

} // namespace scopes_ng
