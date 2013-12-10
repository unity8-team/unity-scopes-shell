/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
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

#ifndef NG_SCOPES_H
#define NG_SCOPES_H

// Qt
#include <QAbstractListModel>
#include <QList>
#include <QThread>

#include <scopes/Runtime.h>
#include <scopes/Registry.h>
#include <scopes/Scope.h>
#include <scopes/ScopeProxyFwd.h>

namespace scopes_ng
{

class Scope;

class Q_DECL_EXPORT Scopes : public QAbstractListModel
{
    Q_OBJECT

    Q_ENUMS(Roles)

    Q_PROPERTY(bool loaded READ loaded NOTIFY loadedChanged)

public:
    explicit Scopes(QObject *parent = 0);
    ~Scopes();

    enum Roles {
        RoleScope,
        RoleId,
        RoleVisible,
        RoleTitle
    };

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    Q_INVOKABLE int rowCount(const QModelIndex& parent = QModelIndex()) const;

    Q_INVOKABLE QVariant get(int row) const;
    Q_INVOKABLE QVariant get(const QString& scope_id) const;

    QHash<int, QByteArray> roleNames() const;

    bool loaded() const;

Q_SIGNALS:
    void loadedChanged(bool loaded);

private Q_SLOTS:
    void populateScopes();
    void discoveryFinished();

private:
    QHash<int, QByteArray> m_roles;
    QList<Scope*> m_scopes;
    QThread* m_listThread;
    bool m_loaded;

    unity::api::scopes::Runtime::UPtr m_scopesRuntime;
};

class ScopeListWorker: public QThread
{
    Q_OBJECT

public:
    void setRuntimeConfig(QString const& config);
    void run() override;
    unity::api::scopes::Runtime::UPtr takeRuntime();
    unity::api::scopes::MetadataMap metadataMap() const;

Q_SIGNALS:
    void discoveryFinished();

private:
    QString m_runtimeConfig;
    unity::api::scopes::Runtime::UPtr m_scopesRuntime;
    unity::api::scopes::MetadataMap m_metadataMap;
};


} // namespace scopes_ng

#endif // NG_SCOPES_H
