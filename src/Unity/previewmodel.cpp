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
#include "previewmodel.h"

// local
#include "collectors.h"
#include "scope.h"
#include "previewwidgetmodel.h"
#include "resultsmodel.h"
#include "utils.h"

// Qt
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace scopes_ng
{

using namespace unity;

PreviewModel::PreviewModel(QObject* parent) :
    unity::shell::scopes::PreviewModelInterface (parent),
    m_loaded(false),
    m_processingAction(false),
    m_delayedClear(false),
    m_widgetColumnCount(1)
{
    // we have one column by default
    PreviewWidgetModel* columnModel = new PreviewWidgetModel(this);
    m_previewWidgetModels.append(columnModel);
}

void PreviewModel::setResult(std::shared_ptr<scopes::Result> const& result)
{
    m_previewedResult = result;
}

bool PreviewModel::event(QEvent* ev)
{
    if (ev->type() == PushEvent::eventType) {
        PushEvent* pushEvent = static_cast<PushEvent*>(ev);

        switch (pushEvent->type()) {
            case PushEvent::PREVIEW:
                processPreviewChunk(pushEvent);
                return true;
            default:
                qWarning("PreviewModel: Unhandled PushEvent type");
                break;
        }
    }

    return false;
}

void PreviewModel::processPreviewChunk(PushEvent* pushEvent)
{
    CollectorBase::Status status;
    scopes::ColumnLayoutList columns;
    scopes::PreviewWidgetList widgets;
    QHash<QString, QVariant> preview_data;

    status = pushEvent->collectPreviewData(columns, widgets, preview_data);
    if (status == CollectorBase::Status::CANCELLED) {
        return;
    }

    if (m_delayedClear) {
        clearAll();
        m_delayedClear = false;

        setProcessingAction(false);
    }

    if (!columns.empty()) {
        setColumnLayouts(columns);
    }
    addWidgetDefinitions(widgets);
    updatePreviewData(preview_data);

    // status in [FINISHED, ERROR]
    if (status != CollectorBase::Status::INCOMPLETE) {
        // FIXME: do something special when preview finishes with error?
        m_loaded = true;
        Q_EMIT loadedChanged();
    }
}

void PreviewModel::setDelayedClear()
{
    m_delayedClear = true;
    // signal right away that the preview is "dirty"
    if (m_loaded) {
        m_loaded = false;
        Q_EMIT loadedChanged();
    }
}

void PreviewModel::clearAll()
{
    // clear column models
    for (int i = 0; i < m_previewWidgetModels.size(); i++) {
        m_previewWidgetModels[i]->clearWidgets();
    }
    m_allData.clear();
    m_columnLayouts.clear();
    m_previewWidgets.clear();
    m_dataToWidgetMap.clear();

    if (m_loaded) {
        m_loaded = false;
        Q_EMIT loadedChanged();
    }
}

void PreviewModel::setWidgetColumnCount(int count)
{
    if (count != m_widgetColumnCount && count > 0) {
        int oldCount = m_widgetColumnCount;
        m_widgetColumnCount = count;

        // clear the existing columns
        for (int i = 0; i < std::min(count, oldCount); i++) {
            m_previewWidgetModels[i]->clearWidgets();
        }
        if (oldCount < count) {
            // create new PreviewWidgetModel(s)
            beginInsertRows(QModelIndex(), oldCount, count - 1);
            for (int i = oldCount; i < count; i++) {
                PreviewWidgetModel* columnModel = new PreviewWidgetModel(this);
                m_previewWidgetModels.append(columnModel);
            }
            endInsertRows();
        } else {
            // remove extra columns
            beginRemoveRows(QModelIndex(), count, oldCount - 1);
            for (int i = oldCount - 1; i >= count; i--) {
                delete m_previewWidgetModels.takeLast();
            }
            endRemoveRows();
        }
        // recalculate which columns do the widgets belong to
        for (int i = 0; i < m_previewWidgets.size(); i++) {
            addWidgetToColumnModel(m_previewWidgets[i]);
        }

        Q_EMIT widgetColumnCountChanged();
    }
}

int PreviewModel::widgetColumnCount() const
{
    return m_widgetColumnCount;
}

bool PreviewModel::loaded() const
{
    return m_loaded;
}

bool PreviewModel::processingAction() const
{
    return m_processingAction;
}

void PreviewModel::setProcessingAction(bool processing)
{
    if (processing != m_processingAction) {
        m_processingAction = processing;
        Q_EMIT processingActionChanged();
    }
}

void PreviewModel::setColumnLayouts(scopes::ColumnLayoutList const& layouts)
{
    if (layouts.empty()) return;

    for (auto it = layouts.begin(); it != layouts.end(); ++it) {
        scopes::ColumnLayout const& layout = *it;
        int numColumns = layout.number_of_columns();
        // build the list
        QList<QStringList> widgetsPerColumn;
        for (int i = 0; i < numColumns; i++) {
            std::vector<std::string> widgetArr(layout.column(i));
            QStringList widgets;
            for (unsigned int j = 0; j < widgetArr.size(); j++) {
                widgets.append(QString::fromStdString(widgetArr[j]));
            }
            widgetsPerColumn.append(widgets);
        }
        m_columnLayouts[numColumns] = widgetsPerColumn;
    }
}

void PreviewModel::addWidgetDefinitions(scopes::PreviewWidgetList const& widgets)
{
    if (widgets.empty()) return;

    for (auto it = widgets.begin(); it != widgets.end(); ++it) {
        scopes::PreviewWidget const& widget = *it;
        QString id(QString::fromStdString(widget.id()));
        QString widget_type(QString::fromStdString(widget.widget_type()));
        QHash<QString, QString> components;
        QVariantMap attributes;

        // collect all components and map their values if present in result
        for (auto const& kv_pair : widget.attribute_mappings()) {
            components[QString::fromStdString(kv_pair.first)] = QString::fromStdString(kv_pair.second);
        }
        processComponents(components, attributes);

        // collect all attributes and their values
        for (auto const& attr_pair : widget.attribute_values()) {
            attributes[QString::fromStdString(attr_pair.first)] = scopeVariantToQVariant(attr_pair.second);
        }

        if (!widget_type.isEmpty()) {
            QList<QSharedPointer<PreviewWidgetData>> collapsedWidgets; // only used if type == 'expandable'
            if (widget_type == "expandable") {
                QList<QSharedPointer<PreviewWidgetData>> widgetData;
                for (auto const w: widget.widgets())
                {
                    QHash<QString, QString> components2;
                    QVariantMap attributes2;
                    // collect all components and map their values if present in result
                    for (auto const& kv_pair : w.attribute_mappings()) {
                        components2[QString::fromStdString(kv_pair.first)] = QString::fromStdString(kv_pair.second);
                    }
                    processComponents(components2, attributes2);

                    // collect all attributes and their values
                    for (auto const& attr_pair : w.attribute_values()) {
                        attributes2[QString::fromStdString(attr_pair.first)] = scopeVariantToQVariant(attr_pair.second);
                    }

                    auto subWidgetData = QSharedPointer<PreviewWidgetData>(new PreviewWidgetData(QString::fromStdString(w.id()), QString::fromStdString(w.widget_type()),
                                components2, attributes2));
                    for (auto attr_it = components2.begin(); attr_it != components2.end(); ++attr_it) {
                        m_dataToWidgetMap.insert(attr_it.value(), subWidgetData.data());
                    }

                    collapsedWidgets.append(subWidgetData);
                    widgetData.append(subWidgetData);
                }

                PreviewWidgetModel* submodel = new PreviewWidgetModel(this);
                submodel->addWidgets(widgetData);
                attributes["widgets"] = QVariant::fromValue(submodel); // insert model of this sub-widget into the outer widget's attributes
            }

            auto preview_data = new PreviewWidgetData(id, widget_type, components, attributes);
            if (collapsedWidgets.size()) {
                preview_data->collapsedWidgets = collapsedWidgets;
            }
            for (auto attr_it = components.begin(); attr_it != components.end(); ++attr_it) {
                m_dataToWidgetMap.insert(attr_it.value(), preview_data);
            }
            QSharedPointer<PreviewWidgetData> widgetData(preview_data);
            m_previewWidgets.append(widgetData);

            addWidgetToColumnModel(widgetData);
        }
    }
}

void PreviewModel::processComponents(QHash<QString, QString> const& components, QVariantMap& out_attributes)
{
    if (components.empty()) return;

    // map from preview data and fallback to result data
    for (auto it = components.begin(); it != components.end(); ++it) {
        QString component_name(it.key());
        QString field_name(it.value());
        // check preview data
        if (m_allData.contains(field_name)) {
            out_attributes[component_name] = m_allData.value(field_name);
        } else if (m_previewedResult && m_previewedResult->contains(field_name.toStdString())) {
            out_attributes[component_name] = scopeVariantToQVariant(m_previewedResult->value(field_name.toStdString()));
        } else {
            // FIXME: should we do this?
            out_attributes[component_name] = QVariant();
        }
    }
}

void PreviewModel::addWidgetToColumnModel(QSharedPointer<PreviewWidgetData> const& widgetData)
{
    int destinationColumnIndex = -1;
    int destinationRowIndex = -1;

    if (m_widgetColumnCount == 1 && !m_columnLayouts.contains(1)) {
        // no need to ask shell in this case, just put all in first column
        destinationColumnIndex = 0;
        destinationRowIndex = -1;
    } else if (m_columnLayouts.contains(m_widgetColumnCount)) {
        QList<QStringList> const& columnLayout = m_columnLayouts.value(m_widgetColumnCount);
        // find the row & col
        for (int i = 0; i < columnLayout.size(); i++) {
            destinationRowIndex = columnLayout[i].indexOf(widgetData->id);
            if (destinationRowIndex >= 0) {
                destinationColumnIndex = i;
                break;
            }
        }
    } else {
      // TODO: ask the shell
      destinationColumnIndex = 0;
    }

    if (destinationColumnIndex >= 0 && destinationColumnIndex < m_previewWidgetModels.size()) {
        PreviewWidgetModel* widgetModel = m_previewWidgetModels.at(destinationColumnIndex);
        widgetModel->insertWidget(widgetData, destinationRowIndex);
    }
}

void PreviewModel::updatePreviewData(QHash<QString, QVariant> const& data)
{
    QSet<PreviewWidgetData*> changedWidgets;
    for (auto it = data.begin(); it != data.end(); ++it) {
        m_allData.insert(it.key(), it.value());
        auto map_it = m_dataToWidgetMap.constFind(it.key());
        while (map_it != m_dataToWidgetMap.constEnd() && map_it.key() == it.key()) {
            changedWidgets.insert(map_it.value());
            ++map_it;
        }
    }

    for (int i = 0; i < m_previewWidgets.size(); i++) {
        PreviewWidgetData* widget = m_previewWidgets.at(i).data();
        if (changedWidgets.contains(widget)) {
            // re-process attributes and emit dataChanged
            processComponents(widget->component_map, widget->data);

            for (int j = 0; j < m_previewWidgetModels.size(); j++) {
                // returns true if the notification was emitted
                if (m_previewWidgetModels[j]->widgetChanged(widget)) {
                    break;
                }
            }
        } else { // check if it's expandable widget
            if (widget->type == "expandable") {
                auto const widgetsModelIt = widget->data.find("widgets");
                if (widgetsModelIt!= widget->data.end() && widgetsModelIt.value().canConvert<PreviewWidgetModel*>()) {
                    for (auto it = widget->collapsedWidgets.begin(); it != widget->collapsedWidgets.end(); it++) {
                        auto subwidget = *it;
                        if (changedWidgets.contains(subwidget.data())) {
                            processComponents(subwidget->component_map, subwidget->data);
                            auto model = widgetsModelIt.value().value<PreviewWidgetModel*>();
                            // returns true if the notification was emitted
                            if (model->widgetChanged(subwidget.data())) {
                                break;
                            }
                        }
                    }
                } else { // this should never happen
                    qWarning() << "Can't convert model to PreviewWidgetModel";
                }
            }
        }
    }
}

PreviewWidgetData* PreviewModel::getWidgetData(QString const& widgetId) const
{
    for (int i = 0; i < m_previewWidgets.size(); i++) {
        PreviewWidgetData* widgetData = m_previewWidgets[i].data();
        if (widgetData->id == widgetId) {
            return widgetData;
        }
    }

    return nullptr;
}

int PreviewModel::rowCount(const QModelIndex&) const
{
    return m_previewWidgetModels.size();
}

QVariant PreviewModel::data(const QModelIndex& index, int role) const
{
    switch (role) {
        case RoleColumnModel:
            return QVariant::fromValue(m_previewWidgetModels.at(index.row()));
        default:
            return QVariant();
    }
}

} // namespace scopes_ng
