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

void PreviewWidgetModel::addReplaceWidget(QSharedPointer<PreviewWidgetData> const& widget, int position)
{
    // If the position of new widget is past m_previewWidgetsOrdered list, then add
    // dummy (null) rows.
    if (position >= m_previewWidgetsOrdered.size()) {
        const int dummyRows = position - m_previewWidgetsOrdered.size();
#ifdef VERBOSE_MODEL_UPDATES
        qDebug() << "PreviewWidgetModel::addReplaceWidget(): adding" << dummyRows << "dummy rows";
#endif
        beginInsertRows(QModelIndex(), m_previewWidgetsOrdered.size(), position);
        for (int i = 0; i<dummyRows; i++) {
            m_previewWidgetsOrdered.append(QSharedPointer<PreviewWidgetData>());
        }
        m_previewWidgetsOrdered.insert(position, widget);
        m_previewWidgetsIndex.insert(widget->id, position);
        endInsertRows();
        
        Q_ASSERT(m_previewWidgetsOrdered.size() - 1 == position);
    } else {
        // Replace existing widget at given position
        auto oldWidget = m_previewWidgetsOrdered.at(position);
#ifdef VERBOSE_MODEL_UPDATES
        qDebug() << "PreviewWidgetModel::addReplaceWidget(): replacing widget at position" << position << "with" << widget->id;
#endif
        m_previewWidgetsOrdered.replace(position, widget);
        if (oldWidget) {
            qDebug() << "PreviewWidgetModel::addReplaceWidget(): replaced widget" << oldWidget->id << "at lookup index" << m_previewWidgetsIndex[oldWidget->id];
            m_previewWidgetsIndex.remove(oldWidget->id);
        }
        m_previewWidgetsIndex.insert(widget->id, position);
        auto const idx = createIndex(position, 0);
        Q_EMIT dataChanged(idx, idx);
    }

#ifdef VERBOSE_MODEL_UPDATES
    dumpLookups("addReplaceWidget");
#endif
}

void PreviewWidgetModel::addWidgets(QList<QSharedPointer<PreviewWidgetData>> const& widgetList)
{
    if (widgetList.size() == 0) return;

    beginInsertRows(QModelIndex(), m_previewWidgetsOrdered.count(), m_previewWidgetsOrdered.size() + widgetList.size() - 1);
    int pos = 0;
    Q_FOREACH(QSharedPointer<PreviewWidgetData> const& w, widgetList) {
        m_previewWidgetsOrdered.append(w);
        m_previewWidgetsIndex.insert(w->id, pos++);
    }
    endInsertRows();
}

void PreviewWidgetModel::updateWidget(QSharedPointer<PreviewWidgetData> const& widget, int row)
{
    auto oldWidget = m_previewWidgetsOrdered.at(row);
    if (oldWidget == nullptr || oldWidget->id != widget->id) {
        qWarning() << "PreviewWidgetModel::updateWidget(): unexpected widget" << widget->id;
        return;
    }

#ifdef VERBOSE_MODEL_UPDATES
    qDebug() << "PreviewWidgetModel::updateWidget(): updating widget" << widget->id << " at row" << row << ", data" << widget->data;
#endif
    m_previewWidgetsOrdered.replace(row, widget);
    auto const idx = createIndex(row, 0);
    Q_EMIT dataChanged(idx, idx);
}

void PreviewWidgetModel::updateWidget(QSharedPointer<PreviewWidgetData> const& updatedWidget)
{
    for (int i = 0; i<m_previewWidgetsOrdered.count(); i++) {
        auto widget = m_previewWidgetsOrdered.at(i);
        if (widget != nullptr && updatedWidget->id == widget->id) {
#ifdef VERBOSE_MODEL_UPDATES
            qDebug() << "PreviewWidgetModel::updateWidget(): updating widget" << widget->id << " at row" << i << ", data" << widget->data;
#endif
            m_previewWidgetsOrdered.replace(i, updatedWidget);
            auto const idx = createIndex(i, 0);
            Q_EMIT dataChanged(idx, idx);
            break;
        }
    }
}

void PreviewWidgetModel::clearWidgets()
{
    beginRemoveRows(QModelIndex(), 0, m_previewWidgetsOrdered.count() - 1);
    m_previewWidgetsOrdered.clear();
    m_previewWidgetsIndex.clear();
    endRemoveRows();
}

bool PreviewWidgetModel::widgetChanged(PreviewWidgetData* widget)
{
    for (int i = 0; i < m_previewWidgetsOrdered.size(); i++) {
        if (m_previewWidgetsOrdered[i].data() == widget) {
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
    const int index = widgetIndex(widget->id);
    if (index >= 0) {
#ifdef VERBOSE_MODEL_UPDATES
        qDebug() << "PreviewWidgetModel::removeWidget(): removing widget" << widget->id << "at row" << index;
#endif
        Q_ASSERT(m_previewWidgetsOrdered.at(index) != nullptr);
        Q_ASSERT(m_previewWidgetsOrdered.at(index)->id == widget->id);
        Q_ASSERT(m_previewWidgetsIndex[widget->id] == index);
        
        beginRemoveRows(QModelIndex(), index, index);
        m_previewWidgetsOrdered.removeAt(index);
        m_previewWidgetsIndex.remove(widget->id);
        // Update index lookup
        for (int i = index; i<m_previewWidgetsOrdered.size(); i++) {
            auto wdata = m_previewWidgetsOrdered.at(i);
            if (wdata) {
                m_previewWidgetsIndex[wdata->id] = i;
            }
        }
        endRemoveRows();
    } else {
        qDebug() << "PreviewWidgetModel::removeWidget(): widget" << widget->id << "doesn't exist in the column model";
    }

#ifdef VERBOSE_MODEL_UPDATES
    dumpLookups("removeWidget");
#endif
}

int PreviewWidgetModel::widgetIndex(QString const &widgetId) const
{
    auto it = m_previewWidgetsIndex.constFind(widgetId);
    if (it != m_previewWidgetsIndex.cend()) {
        return it.value();
    }
    return -1;
}

void PreviewWidgetModel::moveWidget(QSharedPointer<PreviewWidgetData> const& widget, int sourceRow, int destRow)
{
    if (sourceRow == destRow) {
        return;
    }
    if (destRow < 0 || destRow >= m_previewWidgetsOrdered.size()) {
        qWarning() << "PreviewWidgetModel::moveWidget(): invalid destRow" << destRow;
        return;
    }
    if (sourceRow < 0 || sourceRow >= m_previewWidgetsOrdered.size()) {
        qWarning() << "PreviewWidgetModel::moveWidget(): invalid sourceRow" << sourceRow;
        return;
    }

    auto existingWidget = m_previewWidgetsOrdered.at(sourceRow);
    if (existingWidget == nullptr || existingWidget->id != widget->id) {
        qWarning() << "PreviewWidgetModel::moveWidget(): unexpected widget" << widget->id;
        return;
    }
#ifdef VERBOSE_MODEL_UPDATES
    qDebug() << "PreviewWidgetModel::moveWidget(): moving widget" << widget->id << "from" << sourceRow << "to" << destRow;
#endif
    beginMoveRows(QModelIndex(), sourceRow, sourceRow, QModelIndex(), destRow + (destRow > sourceRow ? 1 : 0));
    m_previewWidgetsOrdered.move(sourceRow, destRow);
    // Update m_previewWidgetsIndex lookup
    if (sourceRow > destRow) {
        for (int i = destRow + 1; (i<=sourceRow && i<m_previewWidgetsOrdered.size()); i++) {
            auto widget = m_previewWidgetsOrdered.at(i);
            if (widget) {
                auto it = m_previewWidgetsIndex.find(widget->id);
                if (it != m_previewWidgetsIndex.end()) {
                    it.value() = i;
                }
            }
        }
    } else {
        for (int i = sourceRow; (i<=destRow && i<m_previewWidgetsOrdered.size()); i++) {
            auto widget = m_previewWidgetsOrdered.at(i);
            if (widget) {
                auto it = m_previewWidgetsIndex.find(widget->id);
                if (it != m_previewWidgetsIndex.end()) {
                    it.value() = i;
                }
            }
        }
    }
    m_previewWidgetsIndex.insert(widget->id, destRow);
    endMoveRows();
    
#ifdef VERBOSE_MODEL_UPDATES
    dumpLookups("moveWidget");
#endif
}

int PreviewWidgetModel::rowCount(const QModelIndex&) const
{
    return m_previewWidgetsOrdered.size();
}

QSharedPointer<PreviewWidgetData> PreviewWidgetModel::widget(int index) const
{
    if (index >= 0 && index < m_previewWidgetsOrdered.size()) {
        return m_previewWidgetsOrdered.at(index);
    }
    return QSharedPointer<PreviewWidgetData>();
}

QVariant PreviewWidgetModel::data(const QModelIndex& index, int role) const
{
    const int row = index.row();
    if (row >= m_previewWidgetsOrdered.size())
    {
        qWarning() << "PreviewWidgetModel::data - invalid index" << row << "size"
                << m_previewWidgetsOrdered.size();
        return QVariant();
    }

    auto const widget_data = m_previewWidgetsOrdered.at(row);
    if (!widget_data) {
        return QVariant();
    }
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

void PreviewWidgetModel::dumpLookups(QString const& msg)
{
    qDebug() << "--- Widget lookups dump" << msg << "---";
    for (int i = 0; i<m_previewWidgetsOrdered.size(); i++) {
        auto wdata = m_previewWidgetsOrdered.at(i);
        if (wdata) {
            qDebug() << "Widget" << wdata->id << "at position" << i << ", lookup index" << m_previewWidgetsIndex[wdata->id];
        } else {
            qDebug() << "Empty widget slot at index" << i;
        }
    }
}
    
} // namespace scopes_ng
