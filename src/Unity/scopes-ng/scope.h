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
#include <QMetaType>

// scopes
#include <scopes/Scope.h>

namespace scopes_ng
{

class Categories;

class Scope : public QObject
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
    Q_PROPERTY(bool connected READ connected NOTIFY connectedChanged)
    Q_PROPERTY(Categories* categories READ categories NOTIFY categoriesChanged)
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
    bool connected() const;
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

    Q_INVOKABLE void activate(const QVariant &uri, const QVariant &icon_hint, const QVariant &category,
                              const QVariant &result_type, const QVariant &mimetype, const QVariant &title,
                              const QVariant &comment, const QVariant &dnd_uri, const QVariant &metadata);
    Q_INVOKABLE void preview(const QVariant &uri, const QVariant &icon_hint, const QVariant &category,
                              const QVariant &result_type, const QVariant &mimetype, const QVariant &title,
                              const QVariant &comment, const QVariant &dnd_uri, const QVariant &metadata);
    Q_INVOKABLE void cancelActivation();

    //unity::dash::Scope::Ptr unityScope() const;
    void setProxyObject(unity::api::scopes::ScopeProxy const& proxy);
    void setScopeId(QString const& scope_id);

Q_SIGNALS:
    void idChanged();
    void nameChanged(const std::string&);
    void iconHintChanged(const std::string&);
    void descriptionChanged(const std::string&);
    void searchHintChanged(const std::string&);
    void searchInProgressChanged();
    void visibleChanged(bool);
    void shortcutChanged(const std::string&);
    void connectedChanged(bool);
    void categoriesChanged();
    //void filtersChanged();
    void searchQueryChanged();
    void noResultsHintChanged();
    void formFactorChanged();
    void isActiveChanged(bool);

    // signals triggered by activate(..) or preview(..) requests.
    //void previewReady(Preview *preview);
    void showDash();
    void hideDash();
    void gotoUri(const QString &uri);
    void activated();

    void activateApplication(const QString &desktop);

private Q_SLOTS:

private:
    QString m_scopeId;
    QString m_searchQuery;
    QString m_noResultsHint;
    QString m_formFactor;
    bool m_isActive;
    bool m_searchInProgress;

    unity::api::scopes::ScopeProxy m_proxy;
    unity::api::scopes::QueryCtrlProxy m_lastQuery;
    unity::api::scopes::ReceiverBase::SPtr m_lastReceiver;
    Categories* m_categories;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::Scope*)

#endif // SCOPE_H
