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
#include <QSet>

// scopes
#include <unity/scopes/ActivationResponse.h>
#include <unity/scopes/Result.h>
#include <unity/scopes/Scope.h>
#include <unity/scopes/ScopeMetadata.h>

namespace scopes_ng
{

class Categories;
class PushEvent;
class PreviewStack;

class Q_DECL_EXPORT Scope : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString id READ id NOTIFY idChanged)
    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString iconHint READ iconHint NOTIFY iconHintChanged)
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)
    Q_PROPERTY(QString searchHint READ searchHint NOTIFY searchHintChanged)
    Q_PROPERTY(bool searchInProgress READ searchInProgress NOTIFY searchInProgressChanged)
    Q_PROPERTY(bool visible READ visible NOTIFY visibleChanged)
    Q_PROPERTY(QString shortcut READ shortcut NOTIFY shortcutChanged)
    Q_PROPERTY(scopes_ng::Categories* categories READ categories NOTIFY categoriesChanged)
    //Q_PROPERTY(Filters* filters READ filters NOTIFY filtersChanged)

    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(QString noResultsHint READ noResultsHint WRITE setNoResultsHint NOTIFY noResultsHintChanged)
    Q_PROPERTY(QString formFactor READ formFactor WRITE setFormFactor NOTIFY formFactorChanged)
    Q_PROPERTY(bool isActive READ isActive WRITE setActive NOTIFY isActiveChanged)

public:
    explicit Scope(QObject *parent = 0);
    virtual ~Scope();

    virtual bool event(QEvent* ev) override;

    /* getters */
    QString id() const;
    QString name() const;
    QString iconHint() const;
    QString description() const;
    QString searchHint() const;
    bool visible() const;
    QString shortcut() const;
    bool searchInProgress() const;
    Categories* categories() const;
    //Filters* filters() const;
    QString searchQuery() const;
    QString noResultsHint() const;
    QString formFactor() const;
    bool isActive() const;

    /* setters */
    void setSearchQuery(const QString& search_query);
    void setNoResultsHint(const QString& hint);
    void setFormFactor(const QString& form_factor);
    void setActive(const bool);

    Q_INVOKABLE void activate(QVariant const& result);
    Q_INVOKABLE scopes_ng::PreviewStack* preview(QVariant const& result);
    Q_INVOKABLE void cancelActivation();
    Q_INVOKABLE void closeScope(scopes_ng::Scope* scope);

    void setScopeData(unity::scopes::ScopeMetadata const& data);
    void handleActivation(std::shared_ptr<unity::scopes::ActivationResponse> const&, unity::scopes::Result::SPtr const&);

Q_SIGNALS:
    void idChanged();
    void nameChanged(const std::string&);
    void iconHintChanged(const std::string&);
    void descriptionChanged(const std::string&);
    void searchHintChanged(const std::string&);
    void searchInProgressChanged();
    void visibleChanged(bool);
    void shortcutChanged(const std::string&);
    void categoriesChanged();
    //void filtersChanged();
    void searchQueryChanged();
    void noResultsHintChanged();
    void formFactorChanged();
    void isActiveChanged(bool);

    // signals triggered by activate(..) or preview(..) requests.
    void showDash();
    void hideDash();
    void gotoUri(QString const& uri);
    void activated();
    void previewRequested(QVariant const& result);
    void gotoScope(QString const& scopeId);
    void openScope(scopes_ng::Scope* scope);

    void activateApplication(QString const& desktop);

private Q_SLOTS:
    void flushUpdates();
    void metadataRefreshed();

private:
    void processSearchChunk(PushEvent* pushEvent);
    void processPerformQuery(std::shared_ptr<unity::scopes::ActivationResponse> const& response, bool allowDelayedActivation);

    void processResultSet(QList<std::shared_ptr<unity::scopes::CategorisedResult>>& result_set);
    void dispatchSearch();
    void invalidateLastSearch();

    void activateUri(QString const& uri);

    QString m_scopeId;
    QString m_searchQuery;
    QString m_noResultsHint;
    QString m_formFactor;
    bool m_isActive;
    bool m_searchInProgress;

    unity::scopes::ScopeProxy m_proxy;
    unity::scopes::ScopeMetadata::SPtr m_scopeMetadata;
    unity::scopes::SearchListener::SPtr m_lastSearch;
    unity::scopes::QueryCtrlProxy m_lastSearchQuery;
    unity::scopes::ActivationListener::SPtr m_lastActivation;
    std::shared_ptr<unity::scopes::ActivationResponse> m_delayedActivation;
    Categories* m_categories;
    QTimer m_aggregatorTimer;
    QList<std::shared_ptr<unity::scopes::CategorisedResult>> m_cachedResults;
    QSet<scopes_ng::Scope*> m_tempScopes;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::Scope*)

#endif // NG_SCOPE_H
