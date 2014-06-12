import QtQuick 2.0
import Ubuntu.Components 1.1


CheckBox {
    id: box
    property var properties
    property alias value: box.checked
}
