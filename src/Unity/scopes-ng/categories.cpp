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

// self
#include "categories.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QDebug>

#include <scopes/CategoryRenderer.h>

using namespace unity::api;

namespace scopes_ng {

// FIXME: this should be in a common place
#define CATEGORY_JSON_DEFAULTS R"({"schema-version":1,"template": {"category-layout":"grid","card-layout":"vertical","card-size":"medium","overlay-mode":null,"collapsed-rows":2}, "components": { "title":null, "art": { "aspect-ratio":1.0, "fill-mode":"crop" }, "subtitle":null, "mascot":null, "emblem":null, "old-price":null, "price":null, "alt-price":null, "rating":null, "alt-rating":null, "summary":null }, "resources":{}})"

struct CategoryData
{
    static QJsonValue* DEFAULTS;
    scopes::Category::SCPtr category;
    QJsonValue renderer_template;
    QJsonValue components;

    void setCategory(scopes::Category::SCPtr cat)
    {
        category = cat;
        // lazy init of the defaults
        if (DEFAULTS == nullptr) {
            DEFAULTS = new QJsonValue(QJsonDocument::fromJson(QByteArray(CATEGORY_JSON_DEFAULTS)).object());
        }

        QJsonParseError parseError;
        QJsonDocument category_def = QJsonDocument::fromJson(QByteArray(category->renderer_template().data().c_str()), &parseError);
        if (parseError.error != QJsonParseError::NoError || !category_def.isObject()) {
            qWarning() << "Unable to parse category JSON: %s" << parseError.errorString();
            return;
        }

        QJsonObject category_root = mergeOverrides(*DEFAULTS, category_def.object()).toObject();
        // FIXME: validate the merged json
        renderer_template = category_root.value(QString("template"));
        components = category_root.value(QString("components"));
    }

    QJsonValue mergeOverrides(QJsonValue const& defaultVal, QJsonValue const& overrideVal)
    {
        if (overrideVal.isObject() && defaultVal.isObject()) {
            QJsonObject obj(defaultVal.toObject());
            QJsonObject overrideObj(overrideVal.toObject());
            QJsonObject resultObj;
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                if (overrideObj.contains(it.key())) {
                    resultObj.insert(it.key(), mergeOverrides(it.value(), overrideObj[it.key()]));
                } else {
                    resultObj.insert(it.key(), it.value());
                }
            }
            return resultObj;
        } else if (overrideVal.isString() && (defaultVal.isNull() || defaultVal.isObject())) {
            // special case the expansion of "art": "icon" -> "art": {"field": "icon"}
            QJsonObject resultObj(defaultVal.toObject());
            resultObj.insert("field", overrideVal);
            return resultObj;
        } else if (defaultVal.isNull() && overrideVal.isObject()) {
            return overrideVal;
        } else {
            return overrideVal;
        }
    }
};

QJsonValue* CategoryData::DEFAULTS = nullptr;

Categories::Categories(QObject* parent)
    : QAbstractListModel(parent)
{
    m_roles[Categories::RoleCategoryId] = "categoryId";
    m_roles[Categories::RoleName] = "name";
    m_roles[Categories::RoleIcon] = "icon";
    m_roles[Categories::RoleRawRendererTemplate] = "rawRendererTemplate";
    m_roles[Categories::RoleRenderer] = "renderer";
    m_roles[Categories::RoleComponents] = "components";
    m_roles[Categories::RoleProgressSource] = "progressSource";
    m_roles[Categories::RoleResults] = "results";
    m_roles[Categories::RoleCount] = "count";
}

QHash<int, QByteArray>
Categories::roleNames() const
{
    return m_roles;
}

int Categories::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return m_categories.size();
}

ResultsModel* Categories::lookupCategory(std::string const& category_id)
{
    return m_categoryResults[category_id];
}

void Categories::registerCategory(scopes::Category::SCPtr category, ResultsModel* resultsModel)
{
    // do we already have a category with this id?
    int index = -1;
    for (int i = 0; i < m_categories.size(); i++) {
        if (m_categories[i]->category->id() == category->id()) {
            index = i;
            break;
        }
    }
    if (index >= 0) {
        CategoryData* catData = m_categories[index].data();
        // check if any attributes of the category changed
        QVector<int> changedRoles(collectChangedAttributes(catData->category, category));

        catData->setCategory(category);
        if (changedRoles.size() > 0) {
            QModelIndex changedIndex(this->index(index));
            dataChanged(changedIndex, changedIndex, changedRoles);
        }
    } else {
        CategoryData* catData = new CategoryData;
        catData->setCategory(category);

        auto last_index = m_categories.size();
        beginInsertRows(QModelIndex(), last_index, last_index);
        m_categories.append(QSharedPointer<CategoryData>(catData));
        if (resultsModel == nullptr) {
            resultsModel = new ResultsModel(this);
        }
        resultsModel->setCategoryId(QString::fromStdString(category->id()));
        m_categoryResults[category->id()] = resultsModel;
        endInsertRows();
    }
}

QVector<int> Categories::collectChangedAttributes(scopes::Category::SCPtr old_category, scopes::Category::SCPtr category)
{
    QVector<int> roles;

    if (category->title() != old_category->title()) {
        roles.append(RoleName);
    }
    if (category->icon() != old_category->icon()) {
        roles.append(RoleIcon);
    }
    if (category->renderer_template().data() != old_category->renderer_template().data()) {
        roles.append(RoleRenderer);
        roles.append(RoleComponents);
    }

    return roles;
}

void Categories::updateResultCount(ResultsModel* resultsModel)
{
    auto categoryId = resultsModel->categoryId().toStdString();
    int idx = -1;
    for (int i = 0; i < m_categories.count(); i++) {
        if (m_categories[i]->category->id() == categoryId) {
            idx = i;
            break;
        }
    }
    if (idx < 0) {
        qWarning("unable to update results counts");
        return;
    }

    QVector<int> roles;
    roles.append(RoleCount);

    QModelIndex changedIndex(index(idx));
    dataChanged(changedIndex, changedIndex, roles);
}

void Categories::clearAll()
{
    if (m_categories.count() == 0) return;

    Q_FOREACH(ResultsModel* model, m_categoryResults) {
        model->clearResults();
    }

    QModelIndex changeStart(index(0));
    QModelIndex changeEnd(index(m_categories.count() - 1));
    QVector<int> roles;
    roles.append(RoleCount);
    dataChanged(changeStart, changeEnd, roles);
}

QVariant
Categories::data(const QModelIndex& index, int role) const
{
    CategoryData* catData = m_categories.at(index.row()).data();
    scopes::Category::SCPtr cat(catData->category);
    ResultsModel* resultsModel = m_categoryResults.contains(cat->id()) ? m_categoryResults[cat->id()] : nullptr;

    switch (role) {
        case RoleCategoryId:
            return QString::fromStdString(cat->id());
        case RoleName:
            return QString::fromStdString(cat->title());
        case RoleIcon:
            return QString::fromStdString(cat->icon());
        case RoleRawRendererTemplate:
            return QString::fromStdString(cat->renderer_template().data());
        case RoleRenderer:
            return catData->renderer_template.toVariant();
        case RoleComponents:
             return catData->components.toVariant();
        case RoleProgressSource:
            return QVariant();
        case RoleResults:
            return QVariant::fromValue(resultsModel);
        case RoleCount:
            return QVariant(resultsModel ? resultsModel->rowCount(QModelIndex()) : 0);
        default:
            return QVariant();
    }
}

} // namespace scopes_ng
