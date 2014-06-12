import QtQuick 2.0
import Ubuntu.Components 1.1

OptionSelector {
    id: combo

    property var properties
    property alias value: combo.selectedIndex

    model: properties["values"]
}
