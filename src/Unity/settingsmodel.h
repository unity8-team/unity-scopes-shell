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

#include <unity/SymbolExport.h>
#include <unity/shell/scopes/SettingsModelInterface.h>

#include <QAbstractListModel>
#include <QList>
#include <QSharedPointer>
#include <QSettings>

QT_BEGIN_NAMESPACE
class QDir;
class QTimer;
QT_END_NAMESPACE

namespace scopes_ng
{

class Q_DECL_EXPORT SettingsModel: public unity::shell::scopes::SettingsModelInterface
{
Q_OBJECT

    struct Data
    {
        QString id;
        QString displayName;
        QString type;
        QVariant properties;
        QVariant defaultValue;
        QVariant::Type variantType;

        Data(QString const& id_, QString const& displayName_,
                QString const& type_, QVariant const& properties_,
                QVariant const& defaultValue_, QVariant::Type variantType_) :
                id(id_), displayName(displayName_), type(type_), properties(
                        properties_), defaultValue(defaultValue_), variantType(
                        variantType_)
        {
        }
    };

public:
    explicit SettingsModel(const QDir& configDir, const QString& scopeId,
            const QVariant& settingsDefinitions, QObject* parent = 0,
            int settingsTimeout = 300);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
            override;

    bool setData(const QModelIndex&index, const QVariant& value, int role =
            Qt::EditRole) override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    int count() const override;

    QVariant value(const QString& id) const;

protected Q_SLOTS:
    void settings_timeout();

protected:
    int m_settingsTimeout;

    QList<QSharedPointer<Data>> m_data;

    QMap<QString, QSharedPointer<Data>> m_data_by_id;

    QScopedPointer<QSettings> m_settings;

    QMap<QString, QSharedPointer<QTimer>> m_timers;
};

}

Q_DECLARE_METATYPE(scopes_ng::SettingsModel*)

#endif /* NG_PREVIEW_SETTTINGSMODEL_H_ */
