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
#include <QPointer>

#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/ColumnLayout.h>

namespace scopes_ng
{

struct PreviewWidgetData
{
    QString id;
    QString type;
    QHash<QString, QString> component_map;
    QVariantMap data;

    PreviewWidgetData(QString const& id_, QString const& type_, QHash<QString, QString> const& components, QVariantMap const& data_): id(id_), type(type_), component_map(components), data(data_)
    {
    }
};

class PreviewWidgetModel;
class PushEvent;
class Scope;

class Q_DECL_EXPORT PreviewModel : public QAbstractListModel
{
    Q_OBJECT

    Q_ENUMS(Roles)

    Q_PROPERTY(int widgetColumnCount READ widgetColumnCount WRITE setWidgetColumnCount NOTIFY widgetColumnCountChanged)
    Q_PROPERTY(bool loaded READ loaded NOTIFY loadedChanged)
    Q_PROPERTY(bool processingAction READ processingAction NOTIFY processingActionChanged)

public:
    explicit PreviewModel(QObject* parent = 0);

    enum Roles {
        RoleColumnModel
    };

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    virtual bool event(QEvent* ev) override;

    void setResult(std::shared_ptr<unity::scopes::Result> const&);

    void setWidgetColumnCount(int count);
    int widgetColumnCount() const;
    bool loaded() const;
    bool processingAction() const;
    void setProcessingAction(bool processing);

    void setDelayedClear();
    void clearAll();
    PreviewWidgetData* getWidgetData(QString const& widgetId) const;

Q_SIGNALS:
    void widgetColumnCountChanged();
    void loadedChanged();
    void processingActionChanged();
    void triggered(QString const&, QString const&, QVariantMap const&);

private:
    void processPreviewChunk(PushEvent* pushEvent);
    void setColumnLayouts(unity::scopes::ColumnLayoutList const&);
    void addWidgetDefinitions(unity::scopes::PreviewWidgetList const&);
    void updatePreviewData(QHash<QString, QVariant> const&);
    void addWidgetToColumnModel(QSharedPointer<PreviewWidgetData> const&);
    void processComponents(QHash<QString, QString> const& components, QVariantMap& out_attributes);

    bool m_loaded;
    bool m_processingAction;
    bool m_delayedClear;
    int m_widgetColumnCount;
    QMap<QString, QVariant> m_allData;
    QHash<int, QList<QStringList>> m_columnLayouts;
    QList<PreviewWidgetModel*> m_previewWidgetModels;
    QList<QSharedPointer<PreviewWidgetData>> m_previewWidgets;
    QMultiMap<QString, PreviewWidgetData*> m_dataToWidgetMap;

    std::shared_ptr<unity::scopes::Result> m_previewedResult;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::PreviewModel*)

#endif // NG_PREVIEW_MODEL_H
