// Giriş ve kilit ekranının büyük saati ve tarihi
import QtQuick

import Ankaros.UI

Column {
    id: root

    property date now: new Date()
    property int size: 96

    spacing: 6

    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        text: root.now.toLocaleTimeString(Qt.locale(), Appearance.clockFormat === "12" ? "h:mm AP" : "HH:mm")
        color: "#ffffff"
        font.pixelSize: root.size
        font.weight: Font.DemiBold
        font.letterSpacing: -root.size * 0.04
    }

    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        text: root.now.toLocaleDateString(Qt.locale(), "d MMMM dddd")
        color: Qt.rgba(1, 1, 1, 0.88)
        font.pixelSize: 18
        font.weight: Font.Medium
    }

    Timer {
        interval: 1000
        running: root.visible
        repeat: true
        onTriggered: root.now = new Date()
    }
}
