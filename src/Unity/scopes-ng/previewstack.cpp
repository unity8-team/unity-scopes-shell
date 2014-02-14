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

// local
#include "previewmodel.h"
#include "scope.h"
#include "utils.h"

// Qt
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include <unity/scopes/ActionMetadata.h>
#include <unity/scopes/Scope.h>

namespace scopes_ng
{

using namespace unity;

PreviewStack::PreviewStack(QObject* parent) : QAbstractListModel(parent), m_widgetColumnCount(1)
{
}

PreviewStack::~PreviewStack()
{
    for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it) {
        auto listener = it.value().lock();
        if (listener) listener->invalidate();
    }

    if (m_lastActivation) {
        m_lastActivation->invalidate();
    }
}

QHash<int, QByteArray> PreviewStack::roleNames() const
{
    QHash<int, QByteArray> roles;

    roles[Roles::RolePreviewModel] = "previewModel";

    return roles;
}

bool PreviewStack::event(QEvent* ev)
{
    if (ev->type() == PushEvent::eventType) {
        PushEvent* pushEvent = static_cast<PushEvent*>(ev);

        switch (pushEvent->type()) {
            case PushEvent::ACTIVATION:
                processActionResponse(pushEvent);
                return true;
            default:
                qWarning("PreviewStack: Unhandled PushEvent type");
                break;
        }
    }

    return false;
}

void PreviewStack::setAssociatedScope(scopes_ng::Scope* scope)
{
    m_associatedScope = scope;
}

void PreviewStack::loadForResult(scopes::Result::SPtr const& result)
{
    m_previewedResult = result;

    beginResetModel();

    // invalidate all listeners
    for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it) {
        auto listener = it.value().lock();
        if (listener) listener->invalidate();
    }
    // clear any previews
    while (!m_previews.empty()) {
        delete m_previews.takeFirst();
    }
    // create active preview
    m_activePreview = new PreviewModel(this);
    m_activePreview->setResult(m_previewedResult);
    connect(m_activePreview, &PreviewModel::triggered, this, &PreviewStack::widgetTriggered);
    m_previews.append(m_activePreview);

    endResetModel();

    dispatchPreview();
}

void PreviewStack::dispatchPreview(scopes::Variant const& extra_data)
{
    // TODO: figure out if the result can produce a preview without sending a request to the scope
    // if (m_previewedResult->has_early_preview()) { ... }
    try {
        auto proxy = m_previewedResult->target_scope_proxy();

        scopes::ActionMetadata metadata("C", "phone"); //FIXME
        if (!extra_data.is_null()) {
            metadata.set_scope_data(extra_data);
        }

        std::shared_ptr<PreviewDataReceiver> listener(new PreviewDataReceiver(m_activePreview));
        std::weak_ptr<ScopeDataReceiverBase> wl(listener);
        // invalidate previous listener (if any)
        auto prev_listener = m_listeners.take(m_activePreview).lock();
        if (prev_listener) prev_listener->invalidate();
        m_listeners[m_activePreview] = wl;

        // FIXME: don't block
        m_lastPreviewQuery = proxy->preview(*(m_previewedResult.get()), metadata, listener);
    } catch (std::exception& e) {
        qWarning("Caught an error from preview(): %s", e.what());
    } catch (...) {
        qWarning("Caught an error from preview()");
    }
}

void PreviewStack::widgetTriggered(QString const& widgetId, QString const& actionId, QVariantMap const& data)
{
    try {
        auto proxy = m_previewedResult->target_scope_proxy();
        scopes::ActionMetadata metadata("C", "phone"); //FIXME
        metadata.set_scope_data(qVariantToScopeVariant(data));

        if (m_lastActivation) {
            m_lastActivation->invalidate();
        }
        std::shared_ptr<ActivationReceiver> listener(new ActivationReceiver(this, m_previewedResult));
        m_lastActivation = listener;
        // FIXME: don't block
        proxy->perform_action(*(m_previewedResult.get()), metadata, widgetId.toStdString(), actionId.toStdString(), listener);
    } catch (std::exception& e) {
        qWarning("Caught an error from perform_action(%s, %s): %s", widgetId.toStdString().c_str(), actionId.toStdString().c_str(), e.what());
    } catch (...) {
        qWarning("Caught an error from perform_action()");
    }
}

void PreviewStack::processActionResponse(PushEvent* pushEvent)
{
    std::shared_ptr<scopes::ActivationResponse> response;
    scopes::Result::SPtr result;
    pushEvent->collectActivationResponse(response, result);
    if (!response) return;

    switch (response->status()) {
        case scopes::ActivationResponse::ShowPreview:
            // replace current preview
            dispatchPreview(scopes::Variant(response->hints()));
            break;
        // TODO: case to nest preview (once such API is available)
        default:
            if (m_associatedScope) {
                m_associatedScope->handleActivation(response, result);
            }
            break;
    }
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
