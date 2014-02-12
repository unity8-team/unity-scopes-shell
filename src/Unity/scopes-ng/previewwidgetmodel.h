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

#include <QAbstractListModel>
#include <QSet>
#include <QSharedPointer>
#include <QMultiMap>

#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/Result.h>

#include "previewmodel.h"

namespace scopes_ng
{

class Q_DECL_EXPORT PreviewWidgetModel : public QAbstractListModel
{
    Q_OBJECT

    Q_ENUMS(Roles)

public:
    explicit PreviewWidgetModel(QObject* parent = 0);

    enum Roles {
        RoleWidgetId,
        RoleType,
        RoleProperties
    };

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    void insertWidget(QSharedPointer<PreviewData> const&, int);
    void addWidgets(QList<QSharedPointer<PreviewData>> const&);
    void adoptWidgets(QList<QSharedPointer<PreviewData>> const&);

    void clearWidgets();
    //void updatePreviewData(QHash<QString, QVariant> const&);

Q_SIGNALS:
    void triggered(QString const&, QString const&, QVariantMap const&);

private Q_SLOTS:

private:
    QList<QSharedPointer<PreviewData>> m_previewWidgets;

    std::shared_ptr<unity::scopes::Result> m_previewedResult;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::PreviewWidgetModel*)

#endif // NG_PREVIEW_WIDGET_MODEL_H
