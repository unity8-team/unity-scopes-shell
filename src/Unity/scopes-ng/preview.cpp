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
#include "preview.h"

// local
#include "utils.h"

// Qt
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace scopes_ng
{

using namespace unity;

struct PreviewData
{
    QString id;
    QString type;
    QHash<QString, QString> component_map;
    QVariantMap data;

    PreviewData(QString const& id_, QString const& type_, QHash<QString, QString> const& components, QVariantMap const& data_): id(id_), type(type_), component_map(components), data(data_)
    {
    }
};

PreviewModel::PreviewModel(QObject* parent) : QAbstractListModel(parent)
{
    m_roles[Roles::RoleWidgetId] = "widgetId";
    m_roles[Roles::RoleType] = "type";
    m_roles[Roles::RoleProperties] = "properties";
}

QHash<int, QByteArray> PreviewModel::roleNames() const
{
    return m_roles;
}

void PreviewModel::setResult(std::shared_ptr<scopes::Result> const& result)
{
    m_previewedResult = result;
}

void PreviewModel::addWidgetDefinitions(scopes::PreviewWidgetList const& widgets)
{
    if (widgets.size() == 0) return;

    beginInsertRows(QModelIndex(), m_previewWidgets.count(), m_previewWidgets.count() + widgets.size() - 1);
    for (auto it = widgets.begin(); it != widgets.end(); ++it) {
        // FIXME: the API will expose nicer getters soon, use those!
        QJsonDocument d(QJsonDocument::fromJson(QByteArray((*it).data().c_str())));
        QJsonObject root_obj(d.object());
        QString id;
        QString widget_type;
        QVariantMap attributes;
        QHash<QString, QString> components;
        for (auto it = root_obj.begin(); it != root_obj.end(); ++it) {
            if (it.key() == QLatin1String("id")) {
                id = it.value().toString();
            } else if (it.key() == QLatin1String("type")) {
                widget_type = it.value().toString();
            } else if (it.key() == QLatin1String("components")) {
                QJsonObject components_obj(it.value().toObject());
                for (auto c_it = components_obj.begin(); c_it != components_obj.end(); ++c_it) {
                    if (!c_it.value().toString().isEmpty()) {
                        components[c_it.key()] = c_it.value().toString();
                    }
                }
                processComponents(components, attributes);
            } else {
                QJsonValue v(it.value());
                attributes[it.key()] = v.toVariant();
            }
        }
        if (!widget_type.isEmpty()) {
            auto preview_data = new PreviewData(id, widget_type, components, attributes);
            for (auto attr_it = components.begin(); attr_it != components.end(); ++attr_it) {
                m_dataToWidgetMap.insert(attr_it.value(), preview_data);
            }
            m_previewWidgets.append(QSharedPointer<PreviewData>(preview_data));
        }
    }
    endInsertRows();
}

void PreviewModel::processComponents(QHash<QString, QString> const& components, QVariantMap& out_attributes)
{
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

void PreviewModel::updatePreviewData(QHash<QString, QVariant> const& data)
{
    // TODO: emit dataChanged for relevant widgets, if called after addDefs()
    QSet<PreviewData*> changedWidgets;
    for (auto it = data.begin(); it != data.end(); ++it) {
        m_allData.insert(it.key(), it.value());
        if (m_dataToWidgetMap.contains(it.key())) {
            QSet<PreviewData*> values = m_dataToWidgetMap.values(it.key()).toSet();
            changedWidgets.unite(values);
        }
    }

    for (int i = 0; i < m_previewWidgets.size(); i++) {
        PreviewData* widget = m_previewWidgets.at(i).data();
        if (changedWidgets.contains(widget)) {
            // re-process attributes and emit dataChanged
            processComponents(widget->component_map, widget->data);
            QModelIndex changedIndex(index(i));
            QVector<int> changedRoles;
            changedRoles.append(PreviewModel::RoleProperties);
            dataChanged(changedIndex, changedIndex, changedRoles);
        }
    }
}

int PreviewModel::rowCount(const QModelIndex&) const
{
    return m_previewWidgets.size();
}

QVariant PreviewModel::data(const QModelIndex& index, int role) const
{
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
