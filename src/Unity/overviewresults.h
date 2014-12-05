/*
 * Copyright (C) 2014 Canonical, Ltd.
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


#ifndef NG_OVERVIEW_RESULTS_H
#define NG_OVERVIEW_RESULTS_H

#include <unity/shell/scopes/ResultsModelInterface.h>
#include <unity/scopes/ScopeMetadata.h>

#include <QHash>
#include <QMap>

namespace scopes_ng {

class Q_DECL_EXPORT OverviewResultsModel : public unity::shell::scopes::ResultsModelInterface
{
    Q_OBJECT

public:
    enum ExtraRoles {
        RoleScopeId = unity::shell::scopes::ResultsModelInterface::Roles::RoleBackground + 100
    };

    explicit OverviewResultsModel(QObject* parent = 0);

    void setResults(const QList<unity::scopes::ScopeMetadata::SPtr>& results, const QMap<QString, QString>& scopeIdToName);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    int count() const override;

    QString categoryId() const override;
    void setCategoryId(QString const& id) override;

    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE int scopeIndex(const QString& scopeId) const;

private:
    void updateChildScopes(const unity::scopes::ScopeMetadata::SPtr& scopeMetadata, const QMap<QString, QString>& scopeIdToName);
    QList<unity::scopes::ScopeMetadata::SPtr> m_results;
    QMap<QString, QString> m_childScopes;
};

} // namespace scopes_ng

#endif // NG_OVERVIEW_RESULTS_H
