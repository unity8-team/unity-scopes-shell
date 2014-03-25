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

#include <unity/scopes/Runtime.h>
#include <unity/scopes/Registry.h>
#include <unity/scopes/Scope.h>
#include <unity/scopes/ScopeProxyFwd.h>
#include <unity/scopes/ScopeMetadata.h>

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
    Q_INVOKABLE QVariant get(QString const& scopeId) const;

    Scope* getScopeById(QString const& scopeId) const;
    unity::scopes::ScopeMetadata::SPtr getCachedMetadata(QString const& scopeId) const;
    void refreshScopeMetadata();

    QHash<int, QByteArray> roleNames() const;

    bool loaded() const;

Q_SIGNALS:
    void loadedChanged(bool loaded);
    void metadataRefreshed();

private Q_SLOTS:
    void populateScopes();
    void discoveryFinished();
    void refreshFinished();
    void invalidateScopeResults(QString const&);

private:
    static int LIST_DELAY;

    QHash<int, QByteArray> m_roles;
    QList<Scope*> m_scopes;
    QMap<QString, unity::scopes::ScopeMetadata::SPtr> m_cachedMetadata;
    QThread* m_listThread;
    bool m_loaded;

    unity::scopes::Runtime::SPtr m_scopesRuntime;
};

class ScopeListWorker: public QThread
{
    Q_OBJECT

public:
    void setRuntime(unity::scopes::Runtime::SPtr const& runtime);
    void setRuntimeConfig(QString const& config);
    void run() override;
    unity::scopes::Runtime::SPtr getRuntime() const;
    unity::scopes::MetadataMap metadataMap() const;

Q_SIGNALS:
    void discoveryFinished();

private:
    QString m_runtimeConfig;
    unity::scopes::Runtime::SPtr m_scopesRuntime;
    unity::scopes::MetadataMap m_metadataMap;
};


} // namespace scopes_ng

#endif // NG_SCOPES_H
