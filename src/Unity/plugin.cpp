/*
 * Copyright (C) 2012 Canonical, Ltd.
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
 *
 * Author: Michał Sawicz <michal.sawicz@canonical.com>
 */

// Qt
#include <QQmlContext>
#include <qqml.h>

// self
#include "plugin.h"

// local
#include "scopes.h"
#include "scope.h"
#include "categories.h"
#include "department.h"
#include "resultsmodel.h"
#include "previewstack.h"
#include "previewmodel.h"
#include "previewwidgetmodel.h"

void UnityPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("Unity"));

    // new Scopes classes
    qmlRegisterType<scopes_ng::Scopes>(uri, 0, 2, "Scopes");
    qmlRegisterUncreatableType<unity::shell::scopes::ScopeInterface>(uri, 0, 2, "Scope", "Can't create Scope object in QML. Get them from Scopes instance.");
    qmlRegisterUncreatableType<scopes_ng::Department>(uri, 0, 2, "Department", "Can't create Department object in QML. Get them from Scope instance.");
    qmlRegisterUncreatableType<unity::shell::scopes::CategoriesInterface>(uri, 0, 2, "Categories", "Can't create Categories object in QML. Get them from Scope instance.");
    qmlRegisterUncreatableType<scopes_ng::ResultsModel>(uri, 0, 2, "ResultsModel", "Can't create new ResultsModel in QML. Get them from Categories instance.");
    qmlRegisterUncreatableType<unity::shell::scopes::PreviewModelInterface>(uri, 0, 2, "PreviewModel", "Can't create new PreviewModel in QML. Get them from PreviewStack instance.");
    qmlRegisterUncreatableType<scopes_ng::PreviewWidgetModel>(uri, 0, 2, "PreviewWidgetModel", "Can't create new PreviewWidgetModel in QML. Get them from PreviewModel instance.");
    qmlRegisterUncreatableType<unity::shell::scopes::PreviewStackInterface>(uri, 0, 2, "PreviewStack", "Can't create new PreviewStack in QML. Get them from Scope instance.");
}

void UnityPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    QQmlExtensionPlugin::initializeEngine(engine, uri);
}
