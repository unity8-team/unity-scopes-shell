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


#ifndef NG_PREVIEW_MODEL_H
#define NG_PREVIEW_MODEL_H

#include <QAbstractListModel>
#include <QSet>
#include <QSharedPointer>
#include <QMultiMap>
#include <QStringList>

#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/ColumnLayout.h>

namespace scopes_ng
{

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

class PreviewWidgetModel;
class Scope;

class Q_DECL_EXPORT PreviewModel : public QAbstractListModel
{
    Q_OBJECT

    Q_ENUMS(Roles)

    Q_PROPERTY(int widgetColumnCount READ widgetColumnCount WRITE setWidgetColumnCount NOTIFY widgetColumnCountChanged)

public:
    explicit PreviewModel(QObject* parent = 0);

    enum Roles {
        RoleColumnModel
    };

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    Q_INVOKABLE void triggerAction(QString const& widgetId, QString const& actionId, QVariantMap const& data);

    void setResult(std::shared_ptr<unity::scopes::Result> const&);
    void setAssociatedScope(scopes_ng::Scope*);
    void setWidgetColumnCount(int count);
    int widgetColumnCount();

    void setColumnLayouts(unity::scopes::ColumnLayoutList const&);
    void addWidgetDefinitions(unity::scopes::PreviewWidgetList const&);
    void updatePreviewData(QHash<QString, QVariant> const&);

Q_SIGNALS:
    void widgetColumnCountChanged();
    void triggered(QString const&, QString const&, QVariantMap const&);

private Q_SLOTS:
    void widgetTriggered(QString const&, QString const&, QVariantMap const&);

private:
    void addWidgetToColumnModel(QSharedPointer<PreviewData> const&);
    void processComponents(QHash<QString, QString> const& components, QVariantMap& out_attributes);

    int m_widgetColumnCount;
    QMap<QString, QVariant> m_allData;
    QHash<int, QList<QStringList>> m_columnLayouts;
    QList<PreviewWidgetModel*> m_previewWidgetModels;
    QList<QSharedPointer<PreviewData>> m_previewWidgets;
    QMultiMap<QString, PreviewData*> m_dataToWidgetMap;

    class Private;
    std::shared_ptr<Private> m_priv;

    std::shared_ptr<unity::scopes::Result> m_previewedResult;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::PreviewModel*)

#endif // NG_PREVIEW_MODEL_H
