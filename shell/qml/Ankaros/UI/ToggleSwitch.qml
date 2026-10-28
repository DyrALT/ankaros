// Açma/kapama anahtarı (Figma: Toggle).
import QtQuick

Rectangle {
    id: root

    property bool checked

    signal toggled(bool checked)

    implicitWidth: 40
    implicitHeight: 24
    radius: 12
    color: checked ? Theme.color.accent : Theme.color.surfacePressed

    Behavior on color {
        ColorAnimation { duration: Theme.motion.fast }
    }

    Rectangle {
        width: 18
        height: 18
        radius: 9
        y: 3
        x: root.checked ? root.width - width - 3 : 3
        color: "#ffffff"

        Behavior on x {
            NumberAnimation { duration: Theme.motion.fast; easing.type: Theme.motion.easing }
        }
    }

    TapHandler {
        onTapped: root.toggled(!root.checked)
    }
}
