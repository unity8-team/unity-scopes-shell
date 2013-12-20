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
#include <QHash>
#include <QDebug>

#include <scopes/CategoryRenderer.h>

using namespace unity::api;

namespace scopes_ng {

// FIXME: this should be in a common place
#define CATEGORY_JSON_DEFAULTS R"({"schema-version":1,"template": {"category-layout":"grid","card-layout":"vertical","card-size":"medium","overlay-mode":null,"collapsed-rows":2}, "components": { "title":null, "art": { "aspect-ratio":1.0, "fill-mode":"crop" }, "subtitle":null, "mascot":null, "emblem":null, "old-price":null, "price":null, "alt-price":null, "rating":null, "alt-rating":null, "summary":null }, "resources":{}})"

class CategoryData
{
public:
    CategoryData(scopes::Category::SCPtr const& category): m_resultsModel(nullptr)
    {
        setCategory(category);
    }

    void setCategory(scopes::Category::SCPtr const& category)
    {
        m_category = category;
        m_rawTemplate = category->renderer_template().data();

        parseTemplate(m_rawTemplate, &m_rendererTemplate, &m_components);
    }

    scopes::Category::SCPtr category() const
    {
        return m_category;
    }

    std::string rawTemplate() const
    {
        return m_rawTemplate;
    }

    bool overrideTemplate(std::string const& raw_template)
    {
        QJsonValue components;
        QJsonValue renderer;

        if (parseTemplate(raw_template, &renderer, &components)) {
            m_rawTemplate = raw_template;
            m_rendererTemplate = renderer;
            m_components = components;
            return true;
        }

        return false;
    }

    QJsonValue rendererTemplate() const
    {
        return m_rendererTemplate;
    }

    QJsonValue components() const
    {
        return m_components;
    }

    QHash<QString, QString> getComponentsMapping() const
    {
        QHash<QString, QString> result;
        QJsonObject components_dict = m_components.toObject();
        for (auto it = components_dict.begin(); it != components_dict.end(); ++it) {
            if (it.value().isObject() == false) continue;
            QJsonObject component_dict(it.value().toObject());
            QString fieldName(component_dict.value("field").toString());
            if (fieldName.isEmpty()) continue;
            result[it.key()] = fieldName;
        }

        return result;
    }

    QVector<int> updateAttributes(scopes::Category::SCPtr category)
    {
        QVector<int> roles;

        if (category->title() != m_category->title()) {
            roles.append(Categories::RoleName);
        }
        if (category->icon() != m_category->icon()) {
            roles.append(Categories::RoleIcon);
        }
        if (category->renderer_template().data() != m_rawTemplate) {
            roles.append(Categories::RoleRawRendererTemplate);

            QJsonValue old_renderer(m_rendererTemplate);
            QJsonValue old_components(m_components);

            setCategory(category);

            if (m_rendererTemplate != old_renderer) {
                roles.append(Categories::RoleRenderer);
            }
            if (m_components != old_components) {
                roles.append(Categories::RoleComponents);
            }
        } else {
            setCategory(category);
        }

        return roles;
    }

    void setResultsModel(ResultsModel* model)
    {
        m_resultsModel = model;
    }

    ResultsModel* resultsModel() const
    {
        return m_resultsModel;
    }

private:
    static QJsonValue* DEFAULTS;
    scopes::Category::SCPtr m_category;
    std::string m_rawTemplate;
    QJsonValue m_rendererTemplate;
    QJsonValue m_components;
    ResultsModel* m_resultsModel;

    static bool parseTemplate(std::string const& raw_template, QJsonValue* renderer, QJsonValue* components)
    {
        // lazy init of the defaults
        if (DEFAULTS == nullptr) {
            DEFAULTS = new QJsonValue(QJsonDocument::fromJson(QByteArray(CATEGORY_JSON_DEFAULTS)).object());
        }

        QJsonParseError parseError;
        QJsonDocument category_doc = QJsonDocument::fromJson(QByteArray(raw_template.c_str()), &parseError);
        if (parseError.error != QJsonParseError::NoError || !category_doc.isObject()) {
            qWarning() << "Unable to parse category JSON: %s" << parseError.errorString();
            return false;
        }

        QJsonObject category_root = mergeOverrides(*DEFAULTS, category_doc.object()).toObject();
        // FIXME: validate the merged json
        *renderer = category_root.value(QString("template"));
        *components = category_root.value(QString("components"));

        return true;
    }

    static QJsonValue mergeOverrides(QJsonValue const& defaultVal, QJsonValue const& overrideVal)
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
        if (m_categories[i]->category()->id() == category->id()) {
            index = i;
            break;
        }
    }
    if (index >= 0) {
        CategoryData* catData = m_categories[index].data();
        // check if any attributes of the category changed
        QVector<int> changedRoles(catData->updateAttributes(category));

        if (changedRoles.size() > 0) {
            resultsModel = catData->resultsModel();
            if (resultsModel) {
                resultsModel->setComponentsMapping(catData->getComponentsMapping());
            }
            QModelIndex changedIndex(this->index(index));
            dataChanged(changedIndex, changedIndex, changedRoles);
        }
    } else {
        CategoryData* catData = new CategoryData(category);
        if (resultsModel == nullptr) {
            resultsModel = new ResultsModel(this);
        }
        catData->setResultsModel(resultsModel);

        auto last_index = m_categories.size();
        beginInsertRows(QModelIndex(), last_index, last_index);

        m_categories.append(QSharedPointer<CategoryData>(catData));
        resultsModel->setCategoryId(QString::fromStdString(category->id()));
        resultsModel->setComponentsMapping(catData->getComponentsMapping());
        m_categoryResults[category->id()] = resultsModel;

        endInsertRows();
    }
}

void Categories::updateResultCount(ResultsModel* resultsModel)
{
    int idx = -1;
    for (int i = 0; i < m_categories.count(); i++) {
        if (m_categories[i]->resultsModel() == resultsModel) {
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

bool Categories::overrideCategoryJson(QString const& categoryId, QString const& json)
{
    int idx = -1;
    for (int i = 0; i < m_categories.count(); i++) {
        if (m_categories[i]->category()->id() == categoryId.toStdString()) {
            idx = i;
            break;
        }
    }

    if (idx >= 0) {
        CategoryData* catData = m_categories.at(idx).data();
        if (!catData->overrideTemplate(json.toStdString())) {
            return false;
        }
        if (catData->resultsModel()) {
            catData->resultsModel()->setComponentsMapping(catData->getComponentsMapping());
        }
        QModelIndex changeIndex(index(idx));
        QVector<int> roles;
        roles.append(RoleRawRendererTemplate);
        roles.append(RoleRenderer);
        roles.append(RoleComponents);
        dataChanged(changeIndex, changeIndex, roles);

        return true;
    }

    return false;
}

QVariant
Categories::data(const QModelIndex& index, int role) const
{
    CategoryData* catData = m_categories.at(index.row()).data();
    scopes::Category::SCPtr cat(catData->category());
    ResultsModel* resultsModel = catData->resultsModel();

    switch (role) {
        case RoleCategoryId:
            return QString::fromStdString(cat->id());
        case RoleName:
            return QString::fromStdString(cat->title());
        case RoleIcon:
            return QString::fromStdString(cat->icon());
        case RoleRawRendererTemplate:
            return QString::fromStdString(catData->rawTemplate());
        case RoleRenderer:
            return catData->rendererTemplate().toVariant();
        case RoleComponents:
            return catData->components().toVariant();
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
