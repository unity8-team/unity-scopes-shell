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


#ifndef NG_PREVIEW_WIDGET_MODEL_H
#define NG_PREVIEW_WIDGET_MODEL_H

#include <unity/shell/scopes/PreviewWidgetModelInterface.h>

#include <QMap>
#include <QSharedPointer>

#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/Result.h>

#include "previewmodel.h"

namespace scopes_ng
{

class Q_DECL_EXPORT PreviewWidgetModel : public unity::shell::scopes::PreviewWidgetModelInterface
{
    Q_OBJECT

public:
    explicit PreviewWidgetModel(QObject* parent = 0);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    void addReplaceWidget(QSharedPointer<PreviewWidgetData> const&, int);
    void addWidgets(QList<QSharedPointer<PreviewWidgetData>> const&);
    void updateWidget(QSharedPointer<PreviewWidgetData> const&);
    void updateWidget(QSharedPointer<PreviewWidgetData> const&, int);
    bool widgetChanged(PreviewWidgetData*);
    void removeWidget(QSharedPointer<PreviewWidgetData> const&);
    int widgetIndex(QString const &widgetId) const;
    void moveWidget(QSharedPointer<PreviewWidgetData> const&, int, int);
    QSharedPointer<PreviewWidgetData> widget(int) const;

    void clearWidgets();

private Q_SLOTS:

private:
    void dumpLookups(QString const&);
    QList<QSharedPointer<PreviewWidgetData>> m_previewWidgetsOrdered;
    QMap<QString, int> m_previewWidgetsIndex;

    std::shared_ptr<unity::scopes::Result> m_previewedResult;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::PreviewWidgetModel*)

#endif // NG_PREVIEW_WIDGET_MODEL_H
