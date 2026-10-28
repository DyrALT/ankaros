// Metinli düğme (Figma: Button). Birincil: vurgu renginde, ikincil: yüzey renginde.
import QtQuick

Rectangle {
    id: root

    property alias text: label.text
    property bool primary: true

    signal clicked

    implicitWidth: label.implicitWidth + 32
    implicitHeight: 34
    radius: 10
    opacity: enabled ? 1 : 0.5
    color: primary ? (mouse.pressed ? Qt.darker(Theme.color.accent, 1.15) : Theme.color.accent)
                   : (mouse.pressed ? Theme.color.surfacePressed : Theme.color.surfaceHover)
    border.color: primary ? "transparent" : Theme.color.border

    Label {
        id: label

        anchors.centerIn: parent
        color: root.primary ? Theme.color.textOnAccent : Theme.color.textPrimary
        font.weight: Font.DemiBold
    }

    MouseArea {
        id: mouse

        anchors.fill: parent
        onClicked: root.clicked()
    }
}
