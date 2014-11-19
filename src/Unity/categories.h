/*
 * Copyright (C) 2013 Canonical, Ltd.
 *
 * Authors:
 *  Michał Sawicz <michal.sawicz@canonical.com>
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


#ifndef NG_CATEGORIES_H
#define NG_CATEGORIES_H

#include <unity/shell/scopes/CategoriesInterface.h>

#include <QSharedPointer>
#include <QJsonValue>

#include <unity/scopes/Category.h>

#include "resultsmodel.h"

namespace scopes_ng
{

struct CategoryData;

class Q_DECL_EXPORT Categories : public unity::shell::scopes::CategoriesInterface
{
    Q_OBJECT

public:
    explicit Categories(QObject* parent = 0);

    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;

    Q_INVOKABLE bool overrideCategoryJson(QString const& categoryId, QString const& json) override;
    Q_INVOKABLE void addSpecialCategory(QString const& categoryId, QString const& name, QString const& icon, QString const& rawTemplate, QObject* countObject) override;

    QSharedPointer<ResultsModel> lookupCategory(std::string const& category_id);
    void registerCategory(unity::scopes::Category::SCPtr category, QSharedPointer<ResultsModel> model);
    void updateResultCount(QSharedPointer<ResultsModel> resultsModel);
    void clearAll();

    static bool parseTemplate(std::string const& raw_template, QJsonValue* renderer, QJsonValue* components);

private Q_SLOTS:
    void countChanged();

private:
    int getCategoryIndex(QString const& categoryId) const;
    int getFirstEmptyCategoryIndex() const;

    QList<QSharedPointer<CategoryData>> m_categories;
    QMap<std::string, QSharedPointer<ResultsModel>> m_categoryResults;
    QMap<QObject*, QString> m_countObjects;
};

} // namespace scopes_ng

Q_DECLARE_METATYPE(scopes_ng::Categories*)

#endif // NG_CATEGORIES_H
