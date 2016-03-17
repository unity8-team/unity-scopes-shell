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
#include "resultsmap.h"

namespace scopes_ng {

struct SearchContext
{
    ResultsMap newResultsMap;
    ResultsMap oldResultsMap;
    int lastResultIndex;

    void reset();
};

class Q_DECL_EXPORT ResultsModel : public unity::shell::scopes::ResultsModelInterface
{
    Q_OBJECT

public:
    enum ExtraRoles {
        RoleScopeId = unity::shell::scopes::ResultsModelInterface::Roles::RoleBackground + 100
    };

    explicit ResultsModel(QObject* parent = 0);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;

    void addResults(QList<std::shared_ptr<unity::scopes::CategorisedResult>>&);
    void addUpdateResults(QList<std::shared_ptr<unity::scopes::CategorisedResult>>&);
    void clearResults();

    /* getters */
    QString categoryId() const override;
    int count() const override;

    /* setters */
    void setCategoryId(QString const& id) override;
    void setComponentsMapping(QHash<QString, QString> const& mapping);
    void setMaxAtrributesCount(int count);

    QHash<int, QByteArray> roleNames() const override;
    void updateResult(unity::scopes::Result const& result, unity::scopes::Result const& updatedResult);
    void markNewSearch();
    bool needsPurging() const;

private:
    QVariant componentValue(unity::scopes::Result const* result, std::string const& fieldName) const;
    QVariant attributesValue(unity::scopes::Result const* result) const;

    std::unordered_map<std::string, std::string> m_componentMapping;
    QList<std::shared_ptr<unity::scopes::Result>> m_results;
    QString m_categoryId;
    int m_maxAttributes;
    bool m_purge;
    SearchContext m_search_ctx;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(std::shared_ptr<unity::scopes::Result>)

#endif // NG_CATEGORY_RESULTS_H
