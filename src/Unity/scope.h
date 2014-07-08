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

#include "collectors.h"
#include "departmentnode.h"
#include "department.h"

namespace scopes_ng
{

class Categories;
class PushEvent;
class PreviewStack;
class SettingsModel;

class CollectionController
{
public:
    CollectionController() {}
    ~CollectionController()
    {
        if (m_receiver) {
            m_receiver->invalidate();
        }
        // shouldn't call QueryCtrlProxy->cancel() cause the Runtime might be
        // in the process of being destroyed
    }

    bool isValid()
    {
        return m_listener && m_controller;
    }

    void invalidate()
    {
        if (m_receiver) {
            m_receiver->invalidate();
            m_receiver.reset();
        }
        m_listener.reset();
        if (m_controller) {
            m_controller->cancel();
            m_controller.reset();
        }
    }

    void setListener(unity::scopes::ListenerBase::SPtr const& listener)
    {
        m_listener = listener;
        m_receiver = std::dynamic_pointer_cast<ScopeDataReceiverBase>(listener);
    }

    void setController(unity::scopes::QueryCtrlProxy const& controller)
    {
        m_controller = controller;
    }

private:
    unity::scopes::ListenerBase::SPtr m_listener;
    std::shared_ptr<ScopeDataReceiverBase> m_receiver;
    unity::scopes::QueryCtrlProxy m_controller;
};

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
    unity::shell::scopes::SettingsModelInterface* settings() const override;
    QString searchQuery() const override;
    QString noResultsHint() const override;
    QString formFactor() const override;
    bool isActive() const override;
    QString currentDepartmentId() const override;
    bool hasDepartments() const override;
    QVariantMap customizations() const override;

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
    Q_INVOKABLE void performQuery(QString const& cannedQuery) override;

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
    void setCurrentDepartmentId(QString const& id);
    void processSearchChunk(PushEvent* pushEvent);
    void executeCannedQuery(unity::scopes::CannedQuery const& query, bool allowDelayedActivation);

    void processResultSet(QList<std::shared_ptr<unity::scopes::CategorisedResult>>& result_set);
    void dispatchSearch();
    void invalidateLastSearch();

    static unity::scopes::Department::SCPtr findDepartmentById(unity::scopes::Department::SCPtr const& root, std::string const& id);
    static unity::scopes::Department::SCPtr findUpdateNode(DepartmentNode* node, unity::scopes::Department::SCPtr const& scopeNode);

    QString m_searchQuery;
    QString m_noResultsHint;
    QString m_formFactor;
    QString m_currentDepartmentId;
    QVariantMap m_customizations;
    bool m_isActive;
    bool m_searchInProgress;
    bool m_resultsDirty;
    bool m_delayedClear;
    bool m_hasDepartments;

    std::unique_ptr<CollectionController> m_searchController;
    std::unique_ptr<CollectionController> m_activationController;
    unity::scopes::ScopeProxy m_proxy;
    unity::scopes::ScopeMetadata::SPtr m_scopeMetadata;
    std::shared_ptr<unity::scopes::ActivationResponse> m_delayedActivation;
    unity::scopes::Department::SCPtr m_rootDepartment;
    unity::scopes::Department::SCPtr m_lastRootDepartment;
    QGSettings* m_settings;
    Categories* m_categories;
    QScopedPointer<SettingsModel> m_settingsModel;
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
