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

#include <unity/shell/scopes/ScopesInterface.h>
#include "scope.h"

// Qt
#include <QList>
#include <QThread>
#include <QTimer>
#include <QStringList>
#include <QSharedPointer>
#include <QSet>
#include <QGSettings>

#include <unity/scopes/Runtime.h>
#include <unity/scopes/Registry.h>
#include <unity/scopes/Scope.h>
#include <unity/scopes/ScopeProxyFwd.h>
#include <unity/scopes/ScopeMetadata.h>

class QGSettings;

namespace scopes_ng
{

class LocationService;
class Scope;
class OverviewScope;

class Q_DECL_EXPORT Scopes : public unity::shell::scopes::ScopesInterface
{
    Q_OBJECT
public:
    explicit Scopes(QObject *parent = 0);
    ~Scopes();

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Q_INVOKABLE int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    Q_INVOKABLE unity::shell::scopes::ScopeInterface* getScope(int row) const override;
    Q_INVOKABLE unity::shell::scopes::ScopeInterface* getScope(QString const& scopeId) const override;

    Scope::Ptr getScopeByRow(int row) const;
    Scope::Ptr getScopeById(QString const& scopeId) const;
    unity::scopes::ScopeMetadata::SPtr getCachedMetadata(QString const& scopeId) const;
    QMap<QString, unity::scopes::ScopeMetadata::SPtr> getAllMetadata() const;
    QStringList getFavoriteIds() const;
    Q_INVOKABLE void setFavorite(QString const& scopeId, bool value) override;
    Q_INVOKABLE void moveFavoriteTo(QString const& scopeId, int index) override;

    void refreshScopeMetadata();

    bool loaded() const override;
    int count() const override;
    unity::shell::scopes::ScopeInterface* overviewScope() const override;
    Scope::Ptr overviewScopeSPtr() const;

    QSharedPointer<LocationService> locationService() const;
    QString userAgentString() const;

    Scope::Ptr findTempScope(QString const& id) const;
    void addTempScope(Scope::Ptr const& scope);
    Q_INVOKABLE void closeScope(unity::shell::scopes::ScopeInterface* scope) override;

Q_SIGNALS:
    void metadataRefreshed();

protected:
    virtual QString readPartnerId();

private Q_SLOTS:
    void dashSettingsChanged(QString const &key);
    void processFavoriteScopes();
    void populateScopes();
    void discoveryFinished();
    void refreshFinished();
    void invalidateScopeResults(QString const&);
    void prepopulateFirstScope();
    void prepopulateNextScopes();

    void initPopulateScopes();
    void lsbReleaseFinished();
    void completeDiscoveryFinished();
    void purgeScopesToDelete();
    void scopeRegistryChanged();

private:
    void createUserAgentString();

    static int LIST_DELAY;
    static const int SCOPE_DELETE_DELAY;
    class Priv;

    QList<QSharedPointer<Scope>> m_scopes;
    QList<QSharedPointer<Scope>> m_scopesToDelete;
    bool m_noFavorites;
    QStringList m_favoriteScopes;
    QGSettings* m_dashSettings;
    QMap<QString, unity::scopes::ScopeMetadata::SPtr> m_cachedMetadata;
    QSharedPointer<OverviewScope> m_overviewScope;
    QThread* m_listThread;
    QList<QPair<QString, QString>> m_versions;
    QString m_userAgent;
    bool m_loaded;

    QSharedPointer<LocationService> m_locationService;
    QTimer m_startupQueryTimeout;
    QTimer m_scopesToDeleteTimer;
    QTimer m_registryRefreshTimer;

    unity::scopes::Runtime::SPtr m_scopesRuntime;
    QMap<QString, Scope::Ptr> m_tempScopes;

    std::unique_ptr<Priv> m_priv;
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
