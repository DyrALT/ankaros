// Masaüstüne sağ tıklayınca açılan menü. Açılma yeri ve kapanma Desktop.qml'de.
import QtQuick
import QtQuick.Layouts

import Ankaros.UI

GlassPanel {
    id: root

    // Menü şu an gösteriliyor mu. Kapanırken soluklaşır, görünmez olunca fare olaylarını almaz.
    property bool open: false

    signal wallpaperRequested
    signal appearanceRequested
    signal themeToggleRequested
    signal terminalRequested
    signal lockRequested

    implicitWidth: 240
    implicitHeight: column.implicitHeight + 12
    radius: Theme.radius.md
    tint: Theme.color.surfaceWindow
    opacity: open ? 1 : 0
    visible: opacity > 0

    Behavior on opacity {
        NumberAnimation { duration: Theme.motion.fast }
    }

    // Menünün kenar boşluğuna ve ayraçlarına tıklamak arkaya geçip menüyü kapatmasın
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.AllButtons
    }

    ColumnLayout {
        id: column

        anchors.fill: parent
        anchors.margins: 6
        spacing: 2

        MenuItem {
            icon: "image"
            label: qsTr("Duvar kağıdını değiştir")
            onClicked: root.wallpaperRequested()
        }

        MenuItem {
            icon: "sliders"
            label: qsTr("Görünüm ayarları")
            onClicked: root.appearanceRequested()
        }

        MenuItem {
            icon: "moon"
            label: Theme.dark ? qsTr("Açık temaya geç") : qsTr("Koyu temaya geç")
            onClicked: root.themeToggleRequested()
        }

        MenuSeparator {}

        MenuItem {
            icon: "terminal"
            label: qsTr("Terminal aç")
            onClicked: root.terminalRequested()
        }

        MenuItem {
            icon: "lock"
            label: qsTr("Ekranı kilitle")
            shortcut: qsTr("Süper+L")
            onClicked: root.lockRequested()
        }
    }
}
