/*
 * Copyright (C) 2013 Canonical, Ltd.
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


#ifndef NG_PREVIEW_H
#define NG_PREVIEW_H

#include <QAbstractListModel>
#include <QSet>
#include <QSharedPointer>
#include <QMultiMap>

#include <unity/scopes/PreviewWidget.h>
#include <unity/scopes/Result.h>

namespace scopes_ng
{

class PreviewData;

class Q_DECL_EXPORT PreviewModel : public QAbstractListModel
{
    Q_OBJECT

    Q_ENUMS(Roles)

public:
    explicit PreviewModel(QObject* parent = 0);

    enum Roles {
        RoleWidgetId,
        RoleType,
        RoleProperties
    };

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    void setResult(std::shared_ptr<unity::scopes::Result> const&);

    void addWidgetDefinitions(unity::scopes::PreviewWidgetList const&);
    void updatePreviewData(QHash<QString, QVariant> const&);

private Q_SLOTS:

private:
    void processComponents(QHash<QString, QString> const& components, QVariantMap& out_attributes);

    QHash<int, QByteArray> m_roles;
    QList<QSharedPointer<PreviewData>> m_previewWidgets;
    QMap<QString, QVariant> m_allData;
    QMultiMap<QString, PreviewData*> m_dataToWidgetMap;

    std::shared_ptr<unity::scopes::Result> m_previewedResult;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::PreviewModel*)

#endif // NG_CATEGORIES_H
