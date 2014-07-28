/*
 * Copyright (C) 2013-2014 Canonical, Ltd.
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
 * Authors:
 *  Pete Woods <pete.woods@canonical.com>
 */

import QtQuick 2.0
import Ubuntu.Components 1.1
import Unity 0.2

MainView {
    id: app
    applicationName: "example"
    width: units.gu(40)
    height: units.gu(60)

    Arguments {
        id: args
        defaultArgument.help: "Expects a scope ID"
        defaultArgument.valueNames: ["SCOPE"]
    }

    Scopes {
        id: scopes
        onLoadedChanged: {
            app.scope = scopes.getScope(args.defaultArgument.at(0))
        }
    }

    property var scope: ListModel{}

    function toFileName(type) {
        return "widgets/" + type + "SettingsWidget.qml"
    }

    Page {
        title: "Settings for " + scope.name

        ListView {
            anchors.fill: parent
            spacing: units.gu(1)

            model: scope.settings
            delegate: Loader {
                id: loader
                source: toFileName(type)
                onLoaded: {
                    item.properties = properties
                    item.value = value

                    if (type != "boolean") {
                        loader.width = parent.width
                    }
                    if (type == "list") {
                        loader.height = units.gu(10)
                    }

                    itemValue = Qt.binding(function() { return item.value })
                }
                property var itemValue
                onItemValueChanged: {
                    if (value !== item.value) {
                        value = item.value
                    }
                }
            }
            section.property: "displayName"
            section.delegate: Text {
                width: parent.width
                text: section
            }
        }
    }
}
