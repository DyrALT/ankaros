// Birkaç seçenekten birini seçtiren düğme grubu (ör. Sol/Sağ, Otomatik/Elle).
pragma ComponentBehavior: Bound

import QtQuick

Rectangle {
    id: root

    // [{ value: "left", label: "Sol" }, ...]
    property var options: []
    property string value
    property int segmentWidth: 84

    signal selected(string value)

    implicitWidth: segments.implicitWidth + 6
    implicitHeight: 36
    radius: 10
    color: Theme.color.surfaceHover

    Row {
        id: segments

        anchors.centerIn: parent
        spacing: 2

        Repeater {
            model: root.options

            Rectangle {
                id: segment

                required property var modelData
                readonly property bool current: root.value === modelData.value

                width: Math.max(root.segmentWidth, segmentLabel.implicitWidth + 24)
                height: 30
                radius: Theme.radius.sm
                color: current ? Theme.color.surfaceWindow : "transparent"

                Behavior on color {
                    ColorAnimation { duration: Theme.motion.fast }
                }

                Label {
                    id: segmentLabel

                    anchors.centerIn: parent
                    text: segment.modelData.label
                    color: segment.current ? Theme.color.textPrimary : Theme.color.textSecondary
                    font.weight: segment.current ? Font.DemiBold : Font.Medium
                }

                TapHandler {
                    onTapped: root.selected(segment.modelData.value)
                }
            }
        }
    }
}
