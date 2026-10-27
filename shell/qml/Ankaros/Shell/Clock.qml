// Paneldeki saat ve tarih, sistem diline göre biçimlenir.
import QtQuick

Column {
    id: root

    property date now: new Date()

    Text {
        anchors.right: parent.right
        color: Theme.color.textPrimary
        font.pixelSize: Theme.font.body
        font.weight: Font.DemiBold
        text: root.now.toLocaleTimeString(Qt.locale(), "HH:mm")
    }
    Text {
        anchors.right: parent.right
        color: Theme.color.textSecondary
        font.pixelSize: Theme.font.small
        text: root.now.toLocaleDateString(Qt.locale(), "d MMMM ddd")
    }
    Timer {
        // Dakika değişimini kaçırmamak için saniyede bir güncellenir.
        interval: 1000
        repeat: true
        running: true

        onTriggered: root.now = new Date()
    }
}
