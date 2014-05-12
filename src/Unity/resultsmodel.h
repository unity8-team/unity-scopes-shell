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


#ifndef NG_CATEGORY_RESULTS_H
#define NG_CATEGORY_RESULTS_H

#include <unity/shell/scopes/ResultsModelInterface.h>

#include <QHash>

#include <unity/scopes/CategorisedResult.h>
#include <unordered_map>

namespace scopes_ng {

class Q_DECL_EXPORT ResultsModel : public unity::shell::scopes::ResultsModelInterface
{
    Q_OBJECT

public:
    explicit ResultsModel(QObject* parent = 0);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE QVariant get(int row) const override;

    void addResults(QList<std::shared_ptr<unity::scopes::CategorisedResult>> const&);
    void clearResults();

    /* getters */
    QString categoryId() const override;
    int count() const override;

    /* setters */
    void setCategoryId(QString const& id) override;
    void setComponentsMapping(QHash<QString, QString> const& mapping);

private:
    QVariant componentValue(unity::scopes::CategorisedResult const* result, std::string const& fieldName) const;
    QVariant attributesValue(unity::scopes::CategorisedResult const* result) const;

    QHash<int, QByteArray> m_roles;
    std::unordered_map<std::string, std::string> m_componentMapping;
    QList<std::shared_ptr<unity::scopes::CategorisedResult>> m_results;
    QString m_categoryId;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(std::shared_ptr<unity::scopes::Result>)

#endif // NG_CATEGORY_RESULTS_H
