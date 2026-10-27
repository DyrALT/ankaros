// Paneldeki kare düğme. İçeriği (simge) çocuk olarak verilir.
import QtQuick

Rectangle {
    id: root

    property bool checked: false
    default property alias content: contentItem.data

    signal clicked

    color: mouse.pressed || root.checked ? Theme.color.surfacePressed : mouse.containsMouse
                                           ? Theme.color.surfaceHover : "transparent"
    implicitHeight: Theme.size.panelButton
    implicitWidth: Theme.size.panelButton
    radius: Theme.radius.md

    Behavior on color {
        ColorAnimation {
            duration: Theme.motion.fast
        }
    }

    Item {
        id: contentItem

        anchors.fill: parent
        scale: mouse.pressed ? 0.92 : 1

        Behavior on scale {
            NumberAnimation {
                duration: Theme.motion.fast
                easing.type: Theme.motion.easing
            }
        }
    }
    MouseArea {
        id: mouse

        anchors.fill: parent
        hoverEnabled: true

        onClicked: root.clicked()
    }
}
