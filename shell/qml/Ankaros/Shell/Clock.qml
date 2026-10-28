// Alt çubuktaki saat ve tarih, sistem diline göre biçimlenir. Saat biçimi (24/12) ve tarihin gösterilip
// gösterilmeyeceği Ayarlar › Alt çubuk'tan gelir (Appearance).
import QtQuick

import Ankaros.UI

Column {
    id: root

    property date now: new Date()

    spacing: 1

    Label {
        anchors.right: parent.right
        text: root.now.toLocaleTimeString(Qt.locale(), Appearance.clockFormat === "12" ? "h:mm AP" : "HH:mm")
        font.weight: Font.DemiBold
    }

    Label {
        anchors.right: parent.right
        visible: Appearance.showDate
        text: root.now.toLocaleDateString(Qt.locale(), "d MMMM ddd")
        color: Theme.color.textSecondary
        font.pixelSize: Theme.font.caption
    }

    Timer {
        // Dakika değişimini kaçırmamak için saniyede bir güncellenir.
        interval: 1000
        running: true
        repeat: true
        onTriggered: root.now = new Date()
    }
}
