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
#include "logintoaccount.h"

// Qt
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <unity/scopes/Scope.h>
#include <unity/scopes/ActionMetadata.h>

namespace scopes_ng
{

using namespace unity;

PreviewModel::PreviewModel(QObject* parent) :
    unity::shell::scopes::PreviewModelInterface (parent),
    m_loaded(false),
    m_processingAction(false),
    m_widgetColumnCount(1)
{
    connect(this, &PreviewModel::triggered, this, &PreviewModel::widgetTriggered);

    // we have one column by default
    PreviewWidgetModel* columnModel = new PreviewWidgetModel(this);
    m_previewWidgetModels.append(columnModel);
}

PreviewModel::~PreviewModel()
{
    if (m_listener) {
        m_listener->invalidate();
    }

    if (m_lastActivation) {
        m_lastActivation->invalidate();
    }
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
            case PushEvent::ACTIVATION:
                processActionResponse(pushEvent);
                return true;
            default:
                qWarning("PreviewModel: Unhandled PushEvent type");
                break;
        }
    }

    return unity::shell::scopes::PreviewModelInterface::event(ev);
}

void PreviewModel::setAssociatedScope(scopes_ng::Scope* scope, QUuid const& session_id, QString const& userAgent)
{
    m_associatedScope = scope;
    m_session_id = session_id;
    m_userAgent = userAgent;
}

scopes_ng::Scope* PreviewModel::associatedScope() const
{
    return m_associatedScope;
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

    setProcessingAction(false);

#ifdef VERBOSE_MODEL_UPDATES
    qDebug() << "PreviewModel::processPreviewChunk(): widgets#" << widgets.size();
#endif

    setColumnLayouts(columns);
    addWidgetDefinitions(widgets);
    updatePreviewData(preview_data);

    // status in [FINISHED, ERROR]
    if (status != CollectorBase::Status::INCOMPLETE) {
        // FIXME: do something special when preview finishes with error?
        for (auto it = m_previewWidgets.begin(); it != m_previewWidgets.end(); ) {
            auto widget = it.value();
            if (!widget->received) {
                for (auto model: m_previewWidgetModels) {
                    model->removeWidget(widget);
                }
                m_previewWidgetsOrdered.removeOne(widget);
                it = m_previewWidgets.erase(it);
            } else {
                ++it;
            }
        }

#ifdef VERBOSE_MODEL_UPDATES
        qDebug() << "PreviewModel::processPreviewChunk(): preview complete";
#endif
        Q_ASSERT(m_previewWidgets.size() == m_previewWidgetsOrdered.size());
        m_loaded = true;
        Q_EMIT loadedChanged();
    }
}

void PreviewModel::setWidgetColumnCount(int count)
{
#ifdef VERBOSE_MODEL_UPDATES
    qDebug() << "PreviewModel::setWidgetColumnCount():" << count;
#endif
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
        for (auto it = m_previewWidgetsOrdered.cbegin(); it != m_previewWidgetsOrdered.cend(); it++) {
            addWidgetToColumnModel(*it);
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

void PreviewModel::loadForResult(scopes::Result::SPtr const& result)
{
    m_previewedResult = result;
    if (m_listener) {
        m_listener->invalidate(); // TODO: is this needed?
    }

    dispatchPreview();
}

unity::scopes::Result::SPtr PreviewModel::previewedResult() const
{
    return m_previewedResult;
}

void PreviewModel::update(unity::scopes::PreviewWidgetList const& widgets)
{
    updateWidgetDefinitions(widgets);
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
#ifdef VERBOSE_MODEL_UPDATES
    qDebug() << "PreviewModel::setColumnLayouts()";
#endif    
    if (layouts.empty()) return;

    for (auto it = layouts.begin(); it != layouts.end(); ++it) {
        scopes::ColumnLayout const& layout = *it;
        int numColumns = layout.number_of_columns();
        // build the list
        QList<QStringList> widgetsPerColumn;
        widgetsPerColumn.reserve(numColumns);
        for (int i = 0; i < numColumns; i++) {
            std::vector<std::string> widgetArr(layout.column(i));
            QStringList widgets;
            widgets.reserve(widgetArr.size());
            for (std::size_t j = 0; j < widgetArr.size(); j++) {
                widgets.append(QString::fromStdString(widgetArr[j]));
            }
            widgetsPerColumn.append(widgets);
        }
        m_columnLayouts[numColumns] = widgetsPerColumn;
    }
}

void PreviewModel::addWidgetDefinitions(scopes::PreviewWidgetList const& widgets)
{
    processWidgetDefinitions(widgets, [this](QSharedPointer<PreviewWidgetData> widgetData) {
            auto it = m_previewWidgets.find(widgetData->id);
            if (it != m_previewWidgets.end()) {
                it.value() = widgetData;
            } else {
                m_previewWidgets.insert(widgetData->id, widgetData);
                m_previewWidgetsOrdered.append(widgetData);
            }
            addWidgetToColumnModel(widgetData);
    });
}

void PreviewModel::updateWidgetDefinitions(unity::scopes::PreviewWidgetList const& widgets)
{
    processWidgetDefinitions(widgets, [this](QSharedPointer<PreviewWidgetData> widgetData) {
            auto it = m_previewWidgets.find(widgetData->id);
            if (it != m_previewWidgets.end()) {
                it.value() = widgetData;
                // Update widget with that id in all models
                for (auto model: m_previewWidgetModels) {
                    model->updateWidget(widgetData);
                }
        }
    });
}

void PreviewModel::processWidgetDefinitions(unity::scopes::PreviewWidgetList const& widgets, std::function<void(QSharedPointer<PreviewWidgetData>)> const& processFunc)
{
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
            if (widget_type == QLatin1String("expandable")) {
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
                attributes[QStringLiteral("widgets")] = QVariant::fromValue(submodel); // insert model of this sub-widget into the outer widget's attributes
            }

            auto preview_data = new PreviewWidgetData(id, widget_type, components, attributes);
            if (collapsedWidgets.size()) {
                preview_data->collapsedWidgets = collapsedWidgets;
            }
            for (auto attr_it = components.begin(); attr_it != components.end(); ++attr_it) {
                m_dataToWidgetMap.insert(attr_it.value(), preview_data);
            }
            QSharedPointer<PreviewWidgetData> widgetData(preview_data);

            processFunc(widgetData);
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
#ifdef VERBOSE_MODEL_UPDATES
    qDebug() << "PreviewModel::addWidgetToColumnModel(): processing widget" << widgetData->id;
#endif      
    //
    // Find the column and row based on the column layout definition.
    // If column layout hasn't been defined for current screen setup or
    // widget is not present in the layout, then it should be added after last inserted widget.
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
        if (destinationColumnIndex < 0) {
            qWarning() << "PreviewModel::addWidgetToColumnModel(): widget" << widgetData->id << " not defined in column layouts";
            destinationColumnIndex = 0;
        }
    } else {
      destinationColumnIndex = 0;
    }

    Q_ASSERT(destinationColumnIndex >= 0);
    PreviewWidgetModel* widgetModel = m_previewWidgetModels.at(destinationColumnIndex);
    Q_ASSERT(widgetModel);
    
    // if destinationRowIndex is -1, need to place it after last received
    if (destinationRowIndex == -1) {
        destinationRowIndex = 0;
        auto widget = widgetModel->widget(destinationRowIndex);
        while (widget != nullptr && widget->received) {
            widget = widgetModel->widget(++destinationRowIndex);
        }
    }

    //
    // Place / move widget in the column model
#ifdef VERBOSE_MODEL_UPDATES
    qDebug() << "PreviewModel::addWidgetToColumnModel(): destination for widget" << widgetData->id << "is row" << destinationRowIndex << ", column" << destinationColumnIndex;
#endif    
    const int currentPosition = widgetModel->widgetIndex(widgetData->id);
    if (currentPosition < 0) {
        auto widget = widgetModel->widget(destinationRowIndex);
        while (widget != nullptr && widget->received) {
            widget = widgetModel->widget(++destinationRowIndex);
        }
        widgetModel->insertWidget(widgetData, destinationRowIndex);
    } else {
        if (currentPosition != destinationRowIndex) {
            widgetModel->moveWidget(widgetData, currentPosition, destinationRowIndex);
        }
        // Compare widget content to see if it needs updating.
        // Icon-actions needs to be updated every time because unity8 requires it to properly deal
        // with temporaryIcon.
        if ((widgetData->type == "icon-actions") || (*widgetData != *widgetModel->widget(destinationRowIndex))) {
            widgetModel->updateWidget(widgetData, destinationRowIndex);
        }
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

    for (auto it = m_previewWidgets.begin(); it != m_previewWidgets.end(); it++) {
        PreviewWidgetData* widget = it.value().data();
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
            if (widget->type == QLatin1String("expandable")) {
                auto const widgetsModelIt = widget->data.find(QStringLiteral("widgets"));
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
    auto it = m_previewWidgets.constFind(widgetId);
    if (it != m_previewWidgets.cend()) {
        return it.value().data();
    }
    return nullptr;
}

int PreviewModel::rowCount(const QModelIndex&) const
{
    return m_previewWidgetModels.size();
}

QVariant PreviewModel::data(const QModelIndex& index, int role) const
{
    const int row = index.row();
    if (row >= m_previewWidgetModels.size())
    {
        qWarning() << "PreviewModel::data - invalid index" << row << "size"
                << m_previewWidgetModels.size();
        return QVariant();
    }

    switch (role) {
        case RoleColumnModel:
            return QVariant::fromValue(m_previewWidgetModels.at(row));
        default:
            return QVariant();
    }
}

void PreviewModel::dispatchPreview(scopes::Variant const& extra_data)
{
    qDebug() << "PreviewModel::dispatchPreview()";
    // TODO: figure out if the result can produce a preview without sending a request to the scope
    // if (m_previewedResult->has_early_preview()) { ... }
    try {
        auto proxy = m_associatedScope ? m_associatedScope->proxy_for_result(m_previewedResult) : m_previewedResult->target_scope_proxy();

        QString formFactor(m_associatedScope ? m_associatedScope->formFactor() : QStringLiteral("phone"));
        scopes::ActionMetadata metadata(QLocale::system().name().toStdString(), formFactor.toStdString());
        if (!extra_data.is_null()) {
            metadata.set_scope_data(extra_data);
        }
        if (!m_session_id.isNull()) {
            metadata["session-id"] = uuidToString(m_session_id).toStdString();
        }
        if (!m_userAgent.isEmpty()) {
            metadata["user-agent"] = m_userAgent.toStdString();
        }

        std::shared_ptr<PreviewDataReceiver> listener(new PreviewDataReceiver(this));
        // invalidate previous listener (if any); TODO: is this needed?
        if (m_listener) {
            m_listener->invalidate();
        }
        m_listener = listener;

        if (m_loaded) {
            m_loaded = false;
            Q_EMIT loadedChanged();
        }

        // mark all existing preview widgets as 'not received'
        for (auto it = m_previewWidgets.begin(); it != m_previewWidgets.end(); it++) {
            it.value()->received = false;
        }

        m_lastPreviewQuery = proxy->preview(*(m_previewedResult.get()), metadata, listener);
    } catch (std::exception& e) {
        qWarning("Caught an error from preview(): %s", e.what());
    } catch (...) {
        qWarning("Caught an error from preview()");
    }
}

void PreviewModel::widgetTriggered(QString const& widgetId, QString const& actionId, QVariantMap const& data)
{
    qDebug() << "PreviewModel::widgetTriggered(): widget=" << widgetId << "action=" << actionId << "data=" << data;

    auto action = [this, widgetId, actionId, data]() {
        try {
            auto proxy = m_associatedScope ? m_associatedScope->proxy_for_result(m_previewedResult) : m_previewedResult->target_scope_proxy();

            QString formFactor(m_associatedScope ? m_associatedScope->formFactor() : QStringLiteral("phone"));
            scopes::ActionMetadata metadata(QLocale::system().name().toStdString(), formFactor.toStdString());
            metadata.set_scope_data(qVariantToScopeVariant(data));

            if (m_lastActivation) {
                m_lastActivation->invalidate();
            }
            std::shared_ptr<ActivationReceiver> listener(new ActivationReceiver(this, m_previewedResult));
            m_lastActivation = listener;

            setProcessingAction(true);

            // FIXME: don't block
            proxy->perform_action(*(m_previewedResult.get()), metadata, widgetId.toStdString(), actionId.toStdString(), listener);
        } catch (std::exception& e) {
            qWarning("Caught an error from perform_action(%s, %s): %s", widgetId.toStdString().c_str(), actionId.toStdString().c_str(), e.what());
        } catch (...) {
            qWarning("Caught an error from perform_action()");
        }
    };

    PreviewWidgetData* widgetData = getWidgetData(widgetId);
    if (widgetData != nullptr) {
        QString wtype = widgetData->type;
        auto uriAction = [this, wtype, data, action]() {
            if ((wtype == QLatin1String("actions") || wtype == QLatin1String("icon-actions")) && data.contains(QStringLiteral("uri"))) {
                if (m_associatedScope) {
                    m_associatedScope->activateUri(data.value(QStringLiteral("uri")).toString());
                    return;
                }
            }
            action();
        };

        if (m_associatedScope && widgetData->data.contains(QStringLiteral("online_account_details")))
        {
            QVariantMap details = widgetData->data.value(QStringLiteral("online_account_details")).toMap();
            if (details.contains(QStringLiteral("service_name")) &&
                details.contains(QStringLiteral("service_type")) &&
                details.contains(QStringLiteral("provider_name")) &&
                details.contains(QStringLiteral("login_passed_action")) &&
                details.contains(QStringLiteral("login_failed_action")))
            {
                LoginToAccount *login = new LoginToAccount(details.contains(QStringLiteral("scope_id")) ? details.value(QStringLiteral("scope_id")).toString() : QLatin1String(""),
                                                            details.value(QStringLiteral("service_name")).toString(),
                                                            details.value(QStringLiteral("service_type")).toString(),
                                                            details.value(QStringLiteral("provider_name")).toString(),
                                                            details.value(QStringLiteral("login_passed_action")).toInt(),
                                                            details.value(QStringLiteral("login_failed_action")).toInt(),
                                                            this);
                connect(login, SIGNAL(searchInProgress(bool)), m_associatedScope, SLOT(setSearchInProgress(bool)));
                connect(login, &LoginToAccount::finished, [this, login, uriAction](bool, int action_code_index) {
                    if (action_code_index >= 0 && action_code_index <= scopes::OnlineAccountClient::LastActionCode_)
                    {
                        scopes::OnlineAccountClient::PostLoginAction action_code = static_cast<scopes::OnlineAccountClient::PostLoginAction>(action_code_index);
                        switch (action_code)
                        {
                            case scopes::OnlineAccountClient::DoNothing:
                                return;
                            case scopes::OnlineAccountClient::InvalidateResults:
                                m_associatedScope->invalidateResults();
                                return;
                            default:
                                break;
                        }
                    }
                    uriAction();
                    login->deleteLater();
                });
                login->loginToAccount();
                return; // main execution ends here
            }
        } else {
            uriAction();
        }
    } else {
        qWarning("Action triggered for unknown widget \"%s\"", widgetId.toStdString().c_str());
    }
}

void PreviewModel::processActionResponse(PushEvent* pushEvent)
{
    std::shared_ptr<scopes::ActivationResponse> response;
    scopes::Result::SPtr result;
    QString categoryId;
    pushEvent->collectActivationResponse(response, result, categoryId);
    if (!response) return;

    switch (response->status()) {
        case scopes::ActivationResponse::ShowPreview: // replace current preview
            qDebug() << "PreviewModel::processActionResponse(): ShowPreview";
            // the preview is marked as processing action, leave the flag on until the preview is updated
            dispatchPreview(scopes::Variant(response->scope_data()));
            break;
        // TODO: case to nest preview (once such API is available)
        default:
            if (m_associatedScope) {
                qDebug() << "PreviewModel::processActionResponse(): handleActivation";
                m_associatedScope->handleActivation(response, result);
            }

            setProcessingAction(false);
            break;
    }
}


} // namespace scopes_ng
