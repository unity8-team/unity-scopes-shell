/*
 * Copyright (C) 2011 Canonical, Ltd.
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

#ifndef NG_SCOPE_H
#define NG_SCOPE_H

// Qt
#include <QObject>
#include <QString>
#include <QTimer>
#include <QMetaType>
#include <QPointer>
#include <QMultiMap>
#include <QSet>
#include <QGSettings>

// scopes
#include <unity/scopes/ActivationResponse.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/Scope.h>
#include <unity/scopes/ScopeMetadata.h>
#include <unity/shell/scopes/ScopeInterface.h>

#include "departmentnode.h"
#include "department.h"

namespace scopes_ng
{

class Categories;
class PushEvent;
class PreviewStack;

class Q_DECL_EXPORT Scope : public unity::shell::scopes::ScopeInterface
{
    Q_OBJECT

public:
    explicit Scope(QObject *parent = 0);
    virtual ~Scope();

    virtual bool event(QEvent* ev) override;

    /* getters */
    QString id() const override;
    QString name() const override;
    QString iconHint() const override;
    QString description() const override;
    QString searchHint() const override;
    bool visible() const override;
    QString shortcut() const override;
    bool searchInProgress() const override;
    unity::shell::scopes::CategoriesInterface* categories() const override;
    QString searchQuery() const override;
    QString noResultsHint() const override;
    QString formFactor() const override;
    bool isActive() const override;
    QString currentDepartmentId() const override;
    bool hasDepartments() const override;

    /* setters */
    void setSearchQuery(const QString& search_query) override;
    void setNoResultsHint(const QString& hint) override;
    void setFormFactor(const QString& form_factor) override;
    void setActive(const bool) override;

    Q_INVOKABLE void activate(QVariant const& result) override;
    Q_INVOKABLE unity::shell::scopes::PreviewStackInterface* preview(QVariant const& result) override;
    Q_INVOKABLE void cancelActivation() override;
    Q_INVOKABLE void closeScope(unity::shell::scopes::ScopeInterface* scope) override;
    Q_INVOKABLE unity::shell::scopes::DepartmentInterface* getDepartment(QString const& id) override;
    Q_INVOKABLE void loadDepartment(QString const& id) override;

    void setScopeData(unity::scopes::ScopeMetadata const& data);
    void handleActivation(std::shared_ptr<unity::scopes::ActivationResponse> const&, unity::scopes::Result::SPtr const&);
    void activateUri(QString const& uri);

    bool resultsDirty() const;

public Q_SLOTS:
    void invalidateResults();

Q_SIGNALS:
    void resultsDirtyChanged();

private Q_SLOTS:
    void flushUpdates();
    void metadataRefreshed();
    void internetFlagChanged(QString const& key);
    void departmentModelDestroyed(QObject* obj);

private:
    void startTtlTimer();
    void setSearchInProgress(bool searchInProgress);
    void processSearchChunk(PushEvent* pushEvent);
    void executeCannedQuery(unity::scopes::CannedQuery const& query, bool allowDelayedActivation);

    void processResultSet(QList<std::shared_ptr<unity::scopes::CategorisedResult>>& result_set);
    void dispatchSearch();
    void invalidateLastSearch();

    QString m_searchQuery;
    QString m_noResultsHint;
    QString m_formFactor;
    QString m_currentDepartmentId;
    bool m_isActive;
    bool m_searchInProgress;
    bool m_resultsDirty;
    bool m_delayedClear;
    bool m_hasDepartments;

    unity::scopes::ScopeProxy m_proxy;
    unity::scopes::ScopeMetadata::SPtr m_scopeMetadata;
    unity::scopes::SearchListenerBase::SPtr m_lastSearch;
    unity::scopes::QueryCtrlProxy m_lastSearchQuery;
    unity::scopes::ActivationListenerBase::SPtr m_lastActivation;
    std::shared_ptr<unity::scopes::ActivationResponse> m_delayedActivation;
    unity::scopes::Department::SPtr m_rootDepartment;
    unity::scopes::Department::SPtr m_lastRootDepartment;
    unity::scopes::Department::SPtr m_activeDepartment;
    unity::scopes::Department::SPtr m_lastActiveDepartment;
    QGSettings* m_settings;
    Categories* m_categories;
    QSharedPointer<DepartmentNode> m_departmentTree;
    QTimer m_aggregatorTimer;
    QTimer m_clearTimer;
    QTimer m_invalidateTimer;
    QList<std::shared_ptr<unity::scopes::CategorisedResult>> m_cachedResults;
    QSet<unity::shell::scopes::ScopeInterface*> m_tempScopes;
    QMultiMap<QString, Department*> m_departmentModels;
    QMap<Department*, QString> m_inverseDepartments;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::Scope*)

#endif // NG_SCOPE_H
