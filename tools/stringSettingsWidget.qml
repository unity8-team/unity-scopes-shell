import QtQuick 2.0
import Ubuntu.Components 1.1


TextField {
    id: foo
    property var properties
    property var value: foo.text

    text: properties["currentValue"]
}
