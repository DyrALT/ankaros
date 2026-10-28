// Kullanıcının baş harfi, vurgu renginde daire içinde. Giriş ve kilit ekranında beyaz halkalı.
import QtQuick
import QtQuick.Effects

Item {
    id: root

    property string name
    property real size: 32
    property bool ring: false

    implicitWidth: size
    implicitHeight: size

    RectangularShadow {
        anchors.fill: parent
        visible: root.ring
        radius: root.size / 2
        offset.y: 10
        blur: 28
        color: Qt.rgba(0, 0, 0, 0.28)
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: root.ring ? -3 : 0
        radius: width / 2
        color: Qt.rgba(1, 1, 1, 0.9)
        visible: root.ring
    }

    Rectangle {
        anchors.fill: parent
        radius: root.size / 2
        color: Theme.color.accent

        Label {
            anchors.centerIn: parent
            text: root.name.charAt(0).toLocaleUpperCase()
            color: Theme.color.textOnAccent
            font.pixelSize: Math.round(root.size * 0.42)
            font.weight: Font.DemiBold
        }
    }
}
