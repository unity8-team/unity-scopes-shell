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
                    if (type != "boolean") {
                        loader.width = parent.width
                    }
                    if(type == "list") {
                        loader.height = units.gu(10)
                    }
                }
                property var value: item.value
                onValueChanged: {
                    if (item.properties) {
                        scope.settings.setValue(settingId, value)
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
