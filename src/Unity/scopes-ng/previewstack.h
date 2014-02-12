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


#ifndef NG_PREVIEW_STACK_H
#define NG_PREVIEW_STACK_H

#include <QAbstractListModel>
#include <QSet>
#include <QSharedPointer>
#include <QMultiMap>

#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/Result.h>

namespace scopes_ng
{

class PreviewModel;

class Q_DECL_EXPORT PreviewStack : public QAbstractListModel
{
    Q_OBJECT

    Q_ENUMS(Roles)

    Q_PROPERTY(int widgetColumnCount READ widgetColumnCount WRITE setWidgetColumnCount NOTIFY widgetColumnCountChanged)

public:
    explicit PreviewStack(QObject* parent = 0);
    explicit PreviewStack(PreviewModel* previewModel, QObject* parent = 0);

    enum Roles {
        RolePreviewModel
    };

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    Q_INVOKABLE scopes_ng::PreviewModel* get(int index) const;

    void setResult(std::shared_ptr<unity::scopes::Result> const&);
    void addPreviewModel();

    void setWidgetColumnCount(int columnCount);
    int widgetColumnCount() const;

Q_SIGNALS:
    void widgetColumnCountChanged();

private Q_SLOTS:

private:
    int m_widgetColumnCount;
    QList<PreviewModel*> m_previews;

    std::shared_ptr<unity::scopes::Result> m_previewedResult;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::PreviewStack*)

#endif // NG_PREVIEW_STACK_H
