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

#include <unity/shell/scopes/PreviewModelInterface.h>

#include <QSet>
#include <QSharedPointer>
#include <QMultiMap>
#include <QStringList>
#include <QPointer>
#include <QUuid>

#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/ColumnLayout.h>

#include "collectors.h"

namespace scopes_ng
{

struct PreviewWidgetData
{
    QString id;
    QString type;
    QHash<QString, QString> component_map;
    QVariantMap data;
    QList<QSharedPointer<PreviewWidgetData>> collapsedWidgets; // only used if type == 'expandable'

    PreviewWidgetData(QString const& id_, QString const& type_, QHash<QString, QString> const& components, QVariantMap const& data_): id(id_), type(type_), component_map(components), data(data_)
    {
    }
};

class PreviewWidgetModel;
class PushEvent;
class Scope;

class Q_DECL_EXPORT PreviewModel : public unity::shell::scopes::PreviewModelInterface
{
    Q_OBJECT

public:
    explicit PreviewModel(QObject* parent = 0);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    virtual bool event(QEvent* ev) override;

    void setResult(std::shared_ptr<unity::scopes::Result> const&);

    void setWidgetColumnCount(int count) override;
    int widgetColumnCount() const override;
    bool loaded() const override;
    bool processingAction() const override;
    void setProcessingAction(bool processing);

    void setDelayedClear();
    void clearAll();
    PreviewWidgetData* getWidgetData(QString const& widgetId) const;

    void updateWidgetDefinitions(unity::scopes::PreviewWidgetList const&);

    void loadForResult(unity::scopes::Result::SPtr const&);
    void update(unity::scopes::PreviewWidgetList const&);

    void setAssociatedScope(scopes_ng::Scope*, QUuid const&, QString const&);
    scopes_ng::Scope* associatedScope() const;
    unity::scopes::Result::SPtr previewedResult() const;

private Q_SLOTS:
    void widgetTriggered(QString const&, QString const&, QVariantMap const&);

private:
    void processActionResponse(PushEvent* pushEvent);
    void addWidgetDefinitions(unity::scopes::PreviewWidgetList const&);
    void processWidgetDefinitions(unity::scopes::PreviewWidgetList const&, std::function<void(QSharedPointer<PreviewWidgetData>)> const& processFunc);
    void processPreviewChunk(PushEvent* pushEvent);
    void setColumnLayouts(unity::scopes::ColumnLayoutList const&);
    void updatePreviewData(QHash<QString, QVariant> const&);
    PreviewWidgetModel* createExpandableWidgetModel(unity::scopes::PreviewWidget const&, PreviewWidgetData &);
    void addWidgetToColumnModel(QSharedPointer<PreviewWidgetData> const&);
    void processComponents(QHash<QString, QString> const& components, QVariantMap& out_attributes);
    void dispatchPreview(unity::scopes::Variant const& extra_data = unity::scopes::Variant());

    bool m_loaded;
    bool m_processingAction;
    bool m_delayedClear;
    int m_widgetColumnCount;
    QMap<QString, QVariant> m_allData;
    QHash<int, QList<QStringList>> m_columnLayouts;
    QList<PreviewWidgetModel*> m_previewWidgetModels;
    QList<QSharedPointer<PreviewWidgetData>> m_previewWidgets;
    QMultiMap<QString, PreviewWidgetData*> m_dataToWidgetMap;

    unity::scopes::QueryCtrlProxy m_lastPreviewQuery;
    QPointer<scopes_ng::Scope> m_associatedScope;
    QUuid m_session_id;
    QString m_userAgent;
    std::shared_ptr<unity::scopes::Result> m_previewedResult;
    std::shared_ptr<ScopeDataReceiverBase> m_listener;
    std::shared_ptr<ScopeDataReceiverBase> m_lastActivation;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::PreviewModel*)

#endif // NG_PREVIEW_MODEL_H
