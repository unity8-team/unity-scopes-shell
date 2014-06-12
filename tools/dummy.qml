import QtQuick 2.0
import Ubuntu.Components 1.1
import Unity 0.2

MainView {
    id: app
    applicationName: "example"
    width: units.gu(40)
    height: units.gu(60)

    Scopes {
        id: scopes
        onLoadedChanged: {
            app.scope = scopes.getScope("musicaggregator")
        }
    }

    property var scope: ListModel{}

    Page {
        title: "Settings for " + scope.name

        ListView {
            anchors.fill: parent
            spacing: units.gu(1)

            model: scope.settings
            delegate: Loader {
                id: loader
                source: type + "SettingsWidget.qml"
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
