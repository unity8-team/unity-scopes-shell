/*
 * Copyright (C) 2014 Canonical, Ltd.
 *
 * Authors:
 *  Pete Woods <pete.woods@canonical.com>
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

#ifndef NG_PREVIEW_SETTTINGSMODEL_H_
#define NG_PREVIEW_SETTTINGSMODEL_H_

#include <libu1db-qt5/database.h>
#include <libu1db-qt5/document.h>
#include <unity/SymbolExport.h>
#include <unity/shell/scopes/SettingsModelInterface.h>

#include <QAbstractListModel>
#include <QList>
#include <QSharedPointer>

namespace scopes_ng
{

class SettingsModel: public unity::shell::scopes::SettingsModelInterface
{
Q_OBJECT

    struct Data
    {
        QString id;
        QString displayName;
        QString type;
        QVariantMap data;

        Data(QString const& id_, QString const& displayName_,
                QString const& type_, QVariantMap const& data_)
                : id(id_), displayName(displayName_), type(type_), data(data_)
        {
        }
    };

public:
    explicit SettingsModel(const QString& scopeId, const QByteArray& json,
            QObject* parent = 0);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
            override;

    bool setData(const QModelIndex&index, const QVariant& value, int role =
            Qt::EditRole) override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

protected:
    QList<QSharedPointer<Data>> m_data;

    U1db::Database m_database;

    QMap<QString, QSharedPointer<U1db::Document>> m_documents;
};

}

Q_DECLARE_METATYPE(scopes_ng::SettingsModel*)

#endif /* NG_PREVIEW_SETTTINGSMODEL_H_ */
