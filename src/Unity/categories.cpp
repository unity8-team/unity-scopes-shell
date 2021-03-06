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

// local
#include "utils.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include <QHash>
#include <QDebug>
#include <QPointer>

#include <unity/scopes/CategoryRenderer.h>

using namespace unity;

namespace scopes_ng {

const int MAX_NUMBER_OF_CATEGORIES = 32; // when reached, any excess categories which have no results will be removed

// FIXME: this should be in a common place
#define CATEGORY_JSON_DEFAULTS R"({"schema-version":1,"template": {"category-layout":"grid","card-layout":"vertical","card-size":"small","overlay-mode":null,"collapsed-rows":2}, "components": { "title":null, "art": { "aspect-ratio":1.0 }, "subtitle":null, "social-actions":null, "mascot":null, "emblem":null, "summary":null, "attributes": { "max-count":2 }, "background":null, "overlay-color":null }, "resources":{}})"

class CategoryData
{
public:
    CategoryData(scopes::Category::SCPtr const& category)
    {
        setCategory(category);
    }

    CategoryData(CategoryData const& other) = delete;

    void setCategory(scopes::Category::SCPtr const& category)
    {
        m_category = category;
        m_rawTemplate = category->renderer_template().data();

        parseTemplate(m_rawTemplate, &m_rendererTemplate, &m_components);
    }

    QString categoryId() const
    {
        return m_category ?
            QString::fromStdString(m_category->id()) : m_catId;
    }

    QString title() const
    {
        return m_category ?
            QString::fromStdString(m_category->title()) : m_catTitle;
    }

    QString icon() const
    {
        return m_category ?
            QString::fromStdString(m_category->icon()) : m_catIcon;
    }

    QString headerLink() const
    {
        return m_category && m_category->query() ?
            QString::fromStdString(m_category->query()->to_uri()) : QString();
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
            QString fieldName(component_dict.value(QStringLiteral("field")).toString());
            if (fieldName.isEmpty()) continue;
            result[it.key()] = fieldName;
        }

        return result;
    }

    int getMaxAttributes() const
    {
        QJsonObject components_obj = m_components.toObject();
        QJsonObject attrs_obj = components_obj.value(QStringLiteral("attributes")).toObject();
        QJsonValue max_count_val = attrs_obj.value(QStringLiteral("max-count"));

        return max_count_val.toInt(2);
    }

    QVector<int> updateAttributes(const scopes::Category::SCPtr& category)
    {
        QVector<int> roles;

        if (category->title() != m_category->title()) {
            roles.append(Categories::RoleName);
        }
        if (category->icon() != m_category->icon()) {
            roles.append(Categories::RoleIcon);
        }
        std::string oldQuery;
        std::string newQuery;
        if (m_category->query()) {
            oldQuery = m_category->query()->to_uri();
        }
        if (category->query()) {
            newQuery = category->query()->to_uri();
        }
        if (oldQuery != newQuery) {
            roles.append(Categories::RoleHeaderLink);
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

    void setResultsModel(const QSharedPointer<ResultsModel>& model)
    {
        m_resultsModel = model;
    }

    QSharedPointer<ResultsModel> resultsModel() const
    {
        return m_resultsModel;
    }

    int resultsModelCount() const
    {
        if (m_resultsModel) {
            return m_resultsModel->rowCount(QModelIndex());
        }
        if (m_countObject) {
            QVariant count(m_countObject->property("count"));
            return count.toInt();
        }

        return 0;
    }

    static bool parseTemplate(std::string const& raw_template, QJsonValue* renderer, QJsonValue* components)
    {
        // lazy init of the defaults
        if (DEFAULTS == nullptr) {
            DEFAULTS = new QJsonValue(QJsonDocument::fromJson(QByteArray(CATEGORY_JSON_DEFAULTS)).object());
        }

        QJsonParseError parseError;
        QJsonDocument category_doc = QJsonDocument::fromJson(QByteArray(raw_template.c_str()), &parseError);
        if (parseError.error != QJsonParseError::NoError || !category_doc.isObject()) {
            qWarning() << "Unable to parse category JSON: " << parseError.errorString();
            return false;
        }

        QJsonObject category_root = mergeOverrides(*DEFAULTS, category_doc.object()).toObject();
        // fixup parts we mangle
        QJsonValueRef templateRef = category_root[QStringLiteral("template")];
        QJsonObject templateObj(templateRef.toObject());
        if (templateObj.contains(QStringLiteral("card-background"))) {
            QJsonValueRef cardBackgroundRef = templateObj[QStringLiteral("card-background")];
            if (cardBackgroundRef.isString()) {
                QString background(cardBackgroundRef.toString());
                cardBackgroundRef = QJsonValue::fromVariant(backgroundUriToVariant(background));
                templateRef = templateObj;
            }
        }
        // FIXME: validate the merged json
        *renderer = category_root.value(QStringLiteral("template"));
        *components = category_root.value(QStringLiteral("components"));

        return true;
    }

    scopes::Category::SCPtr m_category;
private:
    static QJsonValue* DEFAULTS;
    QString m_catId;
    QString m_catTitle;
    QString m_catIcon;
    std::string m_rawTemplate;
    QJsonValue m_rendererTemplate;
    QJsonValue m_components;
    QSharedPointer<ResultsModel> m_resultsModel;
    QPointer<QObject> m_countObject;

    static QJsonValue mergeOverrides(QJsonValue const& defaultVal, QJsonValue const& overrideVal)
    {
        if (overrideVal.isObject() && defaultVal.isObject()) {
            QJsonObject obj(defaultVal.toObject());
            QJsonObject overrideObj(overrideVal.toObject());
            QJsonObject resultObj;
            // iterate over the default object keys and merge the values with the overrides
            for (auto it = obj.begin(); it != obj.end(); ++it) {
                if (overrideObj.contains(it.key())) {
                    resultObj.insert(it.key(), mergeOverrides(it.value(), overrideObj[it.key()]));
                } else {
                    resultObj.insert(it.key(), it.value());
                }
            }
            // iterate over overrides keys and add everything that wasn't specified in default object
            for (auto it = overrideObj.begin(); it != overrideObj.end(); ++it) {
                if (!resultObj.contains(it.key())) {
                    resultObj.insert(it.key(), it.value());
                }
            }
            return resultObj;
        } else if (overrideVal.isString() && (defaultVal.isNull() || defaultVal.isObject())) {
            // special case the expansion of "art": "icon" -> "art": {"field": "icon"}
            QJsonObject resultObj(defaultVal.toObject());
            resultObj.insert(QStringLiteral("field"), overrideVal);
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
    : unity::shell::scopes::CategoriesInterface(parent),
    m_categoryIndex(0)
{
}

Categories::~Categories()
{
    m_categories.clear();
    m_categoryResults.clear();
}

int Categories::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    return m_categories.size();
}

QSharedPointer<ResultsModel> Categories::lookupCategory(std::string const& category_id)
{
    return m_categoryResults[category_id];
}

int Categories::getCategoryIndex(QString const& categoryId) const
{
    // there shouldn't be too many categories, linear search should be fine
    for (int i = 0; i < m_categories.size(); i++) {
        if (m_categories[i]->categoryId() == categoryId) {
            return i;
        }
    }

    return -1;
}

void Categories::registerCategory(const scopes::Category::SCPtr& category, QSharedPointer<ResultsModel> resultsModel)
{
    // do we already have a category with this id?
    if (m_registeredCategories.find(category->id()) != m_registeredCategories.end()) {
        return;
    }
    m_registeredCategories.insert(category->id());

    int index = getCategoryIndex(QString::fromStdString(category->id()));
    int emptyIndex = m_categoryIndex++;
    if (index >= 0) {
        // re-registering an existing category will move it after the first non-empty category
        if (emptyIndex < index) {
            QSharedPointer<CategoryData> catData;
            // we could do real move, but the view doesn't like it much
            beginRemoveRows(QModelIndex(), index, index);
            catData = m_categories.takeAt(index);
            endRemoveRows();

            // check if any attributes of the category changed
            QVector<int> changedRoles(catData->updateAttributes(category));
            if (changedRoles.size() > 0) {
                resultsModel = catData->resultsModel();
                if (resultsModel) {
                    resultsModel->setComponentsMapping(catData->getComponentsMapping());
                    resultsModel->setMaxAtrributesCount(catData->getMaxAttributes());
                }
            }
            beginInsertRows(QModelIndex(), emptyIndex, emptyIndex);
            m_categories.insert(emptyIndex, catData);
            endInsertRows();
        } else {
            // the category has already been registered for current search,
            // check if any attributes of the category changed
            QSharedPointer<CategoryData> catData = m_categories[index];
            QVector<int> changedRoles(catData->updateAttributes(category));

            if (changedRoles.size() > 0) {
                resultsModel = catData->resultsModel();
                if (resultsModel) {
                    resultsModel->setComponentsMapping(catData->getComponentsMapping());
                    resultsModel->setMaxAtrributesCount(catData->getMaxAttributes());
                }
                QModelIndex changedIndex(this->index(index));
                dataChanged(changedIndex, changedIndex, changedRoles);
            }
        }
    } else {
        QSharedPointer<CategoryData> catData(new CategoryData(category));
        if (!resultsModel) {
            resultsModel.reset(new ResultsModel());
        }
        catData->setResultsModel(resultsModel);

        beginInsertRows(QModelIndex(), emptyIndex, emptyIndex);

        m_categories.insert(emptyIndex, catData);
        resultsModel->setCategoryId(QString::fromStdString(category->id()));
        resultsModel->setComponentsMapping(catData->getComponentsMapping());
        resultsModel->setMaxAtrributesCount(catData->getMaxAttributes());
        m_categoryResults[category->id()] = resultsModel;

        endInsertRows();
    }

    if (m_categories.count() >= MAX_NUMBER_OF_CATEGORIES) {
        // we register one category at a time, so there can be one excess category at most
        const int index = m_categories.count() - 1;
        auto it = m_categories.begin() + index;
        if ((*it)->resultsModelCount() == 0) {
            qDebug() << "Purging unused category:" << (*it)->categoryId();
            beginRemoveRows(QModelIndex(), index, index);
            m_categoryResults.remove((*it)->categoryId().toStdString());
            for (auto kv = m_countObjects.begin(); kv != m_countObjects.end(); ++kv) {
                if (kv.value() == (*it)->categoryId()) {
                    kv.key()->deleteLater();
                    m_countObjects.erase(kv);
                    break;
                }
            }
            m_categories.erase(it);
            endRemoveRows();
        }
    }
}

void Categories::updateResultCount(const QSharedPointer<ResultsModel>& resultsModel)
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

    Q_FOREACH(QSharedPointer<ResultsModel> model, m_categoryResults) {
        model->clearResults();
    }

    QModelIndex changeStart(index(0));
    QModelIndex changeEnd(index(m_categories.count() - 1));
    QVector<int> roles;
    roles.append(RoleCount);
    dataChanged(changeStart, changeEnd, roles);
}

void Categories::markNewSearch()
{
    m_categoryIndex = 0;
    m_registeredCategories.clear();
    for (auto model: m_categoryResults) {
        model->markNewSearch();
    }
}

void Categories::purgeResults()
{
    QVector<int> roles;
    roles.append(RoleCount);

    for (auto it = m_categoryResults.begin(); it != m_categoryResults.end(); it++) {
        auto model = it.value();
        if (model->needsPurging()) {
            model->clearResults();

            QModelIndex idx(index(getCategoryIndex(QString::fromStdString(it.key()))));
            Q_EMIT dataChanged(idx, idx, roles);
        }
    }
}

bool Categories::parseTemplate(std::string const& raw_template, QJsonValue* renderer, QJsonValue* components)
{
    return CategoryData::parseTemplate(raw_template, renderer, components);
}

bool Categories::overrideCategoryJson(QString const& categoryId, QString const& json)
{
    int idx = getCategoryIndex(categoryId);
    if (idx >= 0) {
        QSharedPointer<CategoryData> catData = m_categories.at(idx);
        if (!catData->overrideTemplate(json.toStdString())) {
            return false;
        }
        if (catData->resultsModel()) {
            catData->resultsModel()->setComponentsMapping(catData->getComponentsMapping());
            catData->resultsModel()->setMaxAtrributesCount(catData->getMaxAttributes());
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

void Categories::countChanged()
{
    QObject* countObject = sender();
    if (countObject != nullptr) {
        QString catId(m_countObjects[countObject]);
        if (catId.isEmpty()) return;
        int idx = getCategoryIndex(catId);
        if (idx < 0) return;

        QVector<int> roles;
        roles.append(RoleCount);

        QModelIndex changedIndex(index(idx));
        dataChanged(changedIndex, changedIndex, roles);
    }
}

void Categories::updateResult(unity::scopes::Result const& result, QString const& categoryId, unity::scopes::Result const& updated_result)
{
    qDebug() << "Categories::updateResult(): update result with uri" << QString::fromStdString(result.uri()) << ", category id" << categoryId;
    for (auto catData: m_categories) {
        if (catData->categoryId() == categoryId) {
            catData->resultsModel()->updateResult(result, updated_result);
            return;
        }
    }
    qWarning() << "Categories::updateResult(): no category with id" << categoryId;
}

QVariant
Categories::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    if (row >= m_categories.size() || row < 0)
    {
        qWarning() << "Categories::data - invalid index" << row << "size"
                << m_categories.size();
        return QVariant();
    }

    const QSharedPointer<CategoryData> &catData = m_categories.at(row);

    if (!catData)
    {
        qWarning() << "Categories::data - invalid category data at" << row << "size"
                        << m_categories.size();
        return QVariant();
    }

    switch (role) {
        case RoleCategoryId:
            return catData->categoryId();
        case RoleName:
            return catData->title();
        case RoleIcon:
            return catData->icon();
        case RoleRawRendererTemplate:
            return QString::fromStdString(catData->rawTemplate());
        case RoleRenderer:
            return catData->rendererTemplate().toVariant();
        case RoleComponents:
            return catData->components().toVariant();
        case RoleHeaderLink:
            return catData->headerLink();
        case RoleResults:
        {
            QSharedPointer<ResultsModel> resultsModel = catData->resultsModel();
            if (resultsModel)
            {
                return QVariant::fromValue(resultsModel.data());
            }
            else
            {
                qWarning() << "Category data has no results model" << catData->categoryId();
                return QVariant();
            }
        }
        case RoleCount:
            return catData->resultsModelCount();
        case RoleResultsSPtr:
        {
            QSharedPointer<unity::shell::scopes::ResultsModelInterface> resultsModel = catData->resultsModel();
            if (resultsModel)
            {
                return QVariant::fromValue(resultsModel);
            }
            else
            {
                return QVariant();
            }
        }
        case RoleCategorySPtr:
        {
            if (catData->m_category)
            {
                return QVariant::fromValue(catData->m_category);
            }
            else
            {
                return QVariant();
            }
        }
        default:
            return QVariant();
    }
}

} // namespace scopes_ng
