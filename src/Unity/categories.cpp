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

// FIXME: this should be in a common place
#define CATEGORY_JSON_DEFAULTS R"({"schema-version":1,"template": {"category-layout":"grid","card-layout":"vertical","card-size":"small","overlay-mode":null,"collapsed-rows":2}, "components": { "title":null, "art": { "aspect-ratio":1.0, "fill-mode":"crop" }, "subtitle":null, "mascot":null, "emblem":null, "summary":null, "attributes": { "max-count":2 }, "background":null, "overlay-color":null }, "resources":{}})"

class CategoryData
{
public:
    CategoryData(scopes::Category::SCPtr const& category): m_resultsModel(nullptr), m_isSpecial(false)
    {
        setCategory(category);
    }

    // constructor for special (shell-overriden) categories
    CategoryData(QString const& id, QString const& title, QString const& icon, QString rawTemplate, QObject* countObject):
        m_catId(id), m_catTitle(title), m_catIcon(icon), m_rawTemplate(rawTemplate.toStdString()), m_resultsModel(nullptr), m_countObject(countObject), m_isSpecial(true)
    {
        parseTemplate(m_rawTemplate, &m_rendererTemplate, &m_components);
    }

    ~CategoryData()
    {
        if (m_resultsModel) {
            delete m_resultsModel;
        }
    }

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
            QString fieldName(component_dict.value("field").toString());
            if (fieldName.isEmpty()) continue;
            result[it.key()] = fieldName;
        }

        return result;
    }

    int getMaxAttributes() const
    {
        QJsonObject components_obj = m_components.toObject();
        QJsonObject attrs_obj = components_obj.value("attributes").toObject();
        QJsonValue max_count_val = attrs_obj.value("max-count");

        return max_count_val.toInt(2);
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

    void setResultsModel(ResultsModel* model)
    {
        m_resultsModel = model;
    }

    ResultsModel* resultsModel() const
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

    bool isSpecial() const
    {
        return m_isSpecial;
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
        QJsonValueRef templateRef = category_root["template"];
        QJsonObject templateObj(templateRef.toObject());
        if (templateObj.contains("card-background")) {
            QJsonValueRef cardBackgroundRef = templateObj["card-background"];
            if (cardBackgroundRef.isString()) {
                QString background(cardBackgroundRef.toString());
                cardBackgroundRef = QJsonValue::fromVariant(backgroundUriToVariant(background));
                templateRef = templateObj;
            }
        }
        // FIXME: validate the merged json
        *renderer = category_root.value(QString("template"));
        *components = category_root.value(QString("components"));

        return true;
    }

private:
    static QJsonValue* DEFAULTS;
    scopes::Category::SCPtr m_category;
    QString m_catId;
    QString m_catTitle;
    QString m_catIcon;
    std::string m_rawTemplate;
    QJsonValue m_rendererTemplate;
    QJsonValue m_components;
    ResultsModel* m_resultsModel;
    QPointer<QObject> m_countObject;
    bool m_isSpecial;

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
    : unity::shell::scopes::CategoriesInterface(parent)
{
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

int Categories::getFirstEmptyCategoryIndex() const
{
    for (int i = 0; i < m_categories.size(); i++) {
        if (m_categories[i]->isSpecial()) {
            continue;
        }
        if (m_categories[i]->resultsModelCount() == 0) {
            return i;
        }
    }

    return m_categories.size();
}

void Categories::registerCategory(scopes::Category::SCPtr category, ResultsModel* resultsModel)
{
    // do we already have a category with this id?
    int index = getCategoryIndex(QString::fromStdString(category->id()));
    int emptyIndex = getFirstEmptyCategoryIndex();
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
            CategoryData* catData = m_categories[index].data();
            // check if any attributes of the category changed
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
        CategoryData* catData = new CategoryData(category);
        if (resultsModel == nullptr) {
            resultsModel = new ResultsModel(this);
        }
        catData->setResultsModel(resultsModel);

        beginInsertRows(QModelIndex(), emptyIndex, emptyIndex);

        m_categories.insert(emptyIndex, QSharedPointer<CategoryData>(catData));
        resultsModel->setCategoryId(QString::fromStdString(category->id()));
        resultsModel->setComponentsMapping(catData->getComponentsMapping());
        resultsModel->setMaxAtrributesCount(catData->getMaxAttributes());
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


bool Categories::parseTemplate(std::string const& raw_template, QJsonValue* renderer, QJsonValue* components)
{
    return CategoryData::parseTemplate(raw_template, renderer, components);
}

bool Categories::overrideCategoryJson(QString const& categoryId, QString const& json)
{
    int idx = getCategoryIndex(categoryId);
    if (idx >= 0) {
        CategoryData* catData = m_categories.at(idx).data();
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

void Categories::addSpecialCategory(QString const& categoryId, QString const& name, QString const& icon, QString const& rawTemplate, QObject* countObject)
{
    int index = getCategoryIndex(categoryId);
    if (index >= 0) {
        qWarning("ERROR! Category with id \"%s\" already exists!", categoryId.toStdString().c_str());
    } else {
        CategoryData* catData = new CategoryData(categoryId, name, icon, rawTemplate, countObject);
        // prepend the category
        beginInsertRows(QModelIndex(), 0, 0);
        m_categories.prepend(QSharedPointer<CategoryData>(catData));
        endInsertRows();

        if (countObject) {
            m_countObjects[countObject] = categoryId;
            QObject::connect(countObject, SIGNAL(countChanged()), this, SLOT(countChanged()));
        }
    }
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

QVariant
Categories::data(const QModelIndex& index, int role) const
{
    CategoryData* catData = m_categories.at(index.row()).data();
    ResultsModel* resultsModel = catData->resultsModel();

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
            return QVariant::fromValue(resultsModel);
        case RoleCount:
            return catData->resultsModelCount();
        default:
            return QVariant();
    }
}

} // namespace scopes_ng
