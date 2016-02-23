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
#include <unity/scopes/ChildScope.h>
#include <unity/scopes/Scope.h>
#include <unity/scopes/ScopeMetadata.h>
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
            const QVariant& settingsDefinitions, bool isLocationGloballyEnabled = true,
            QObject* parent = 0,
            int settingsTimeout = 300);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const
            override;

    bool setData(const QModelIndex&index, const QVariant& value, int role =
            Qt::EditRole) override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    int count() const override;

    QVariant value(const QString& id) const;

    void update_child_scopes(QMap<QString, unity::scopes::ScopeMetadata::SPtr> const& scopes_metadata);

    bool require_child_scopes_refresh() const;

Q_SIGNALS:
    void settingsChanged();

protected Q_SLOTS:
    void settings_timeout();

protected:
    QString m_scopeId;
    unity::scopes::ScopeProxy m_scopeProxy;
    int m_settingsTimeout;

    QList<QSharedPointer<Data>> m_data;
    QMap<QString, QSharedPointer<Data>> m_data_by_id;
    QScopedPointer<QSettings> m_settings;
    QMap<QString, QSharedPointer<QTimer>> m_timers;

    QList<QSharedPointer<Data>> m_child_scopes_data;
    QMap<QString, QSharedPointer<Data>> m_child_scopes_data_by_id;
    unity::scopes::ChildScopeList m_child_scopes;
    QMap<QString, QSharedPointer<QTimer>> m_child_scopes_timers;
    bool m_requireChildScopesRefresh;
};

}

Q_DECLARE_METATYPE(scopes_ng::SettingsModel*)

#endif /* NG_PREVIEW_SETTTINGSMODEL_H_ */
