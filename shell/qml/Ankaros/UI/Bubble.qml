// Açıklama balonu. Bilgi simgesinin (InfoTip) ve simge düğmelerinin (GlassIconButton) üzerine gelince
// görünür. Sayfaya açıklama yazısı yazılmaz, açıklamalar burada durur (docs/tasarim-akisi.md).
import QtQuick
import QtQuick.Effects

Item {
    id: root

    property alias text: label.text
    property bool shown: false
    property real maxWidth: 260

    width: Math.min(maxWidth, label.implicitWidth + 24)
    height: label.implicitHeight + 18
    z: 1000
    visible: opacity > 0
    opacity: shown ? 1 : 0

    Behavior on opacity {
        NumberAnimation { duration: Theme.motion.fast }
    }

    RectangularShadow {
        anchors.fill: parent
        radius: 10
        offset.y: 6
        blur: 20
        color: Qt.rgba(0, 0, 0, 0.16)
    }

    Rectangle {
        anchors.fill: parent
        radius: 10
        color: Theme.color.surfaceWindow
        border.color: Theme.color.border
    }

    Label {
        id: label

        x: 12
        y: 9
        width: root.width - 24
        wrapMode: Text.Wrap
        font.pixelSize: Theme.font.small
        lineHeight: 1.35
    }
}
