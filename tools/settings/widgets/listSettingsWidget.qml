import QtQuick 2.0
import Ubuntu.Components 1.1

OptionSelector {
    id: combo

    property var values
    property alias value: combo.selectedIndex

    model: values
}
