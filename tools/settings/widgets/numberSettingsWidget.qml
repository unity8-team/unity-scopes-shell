import QtQuick 2.0
import Ubuntu.Components 1.1


TextField {
    id: field
    property var properties
    property alias value: field.text
    validator: DoubleValidator {}
}
