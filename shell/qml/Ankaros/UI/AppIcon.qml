// Uygulama simgesi: degrade bir kare üzerinde beyaz (açık renkli simgelerde koyu) çizgi simge.
// Renkler ve simge IconData.apps tablosundan uygulama adına göre seçilir.
import QtQuick

import "IconData.js" as IconData

Rectangle {
    id: root

    property string app
    property real size: 64

    readonly property var style: IconData.apps[app] ?? IconData.fallbackApp

    implicitWidth: size
    implicitHeight: size
    radius: size * 0.234
    border.color: Qt.rgba(1, 1, 1, 0.18)
    border.width: 1

    gradient: Gradient {
        GradientStop {
            position: 0
            color: root.style.top
        }
        GradientStop {
            position: 1
            color: root.style.bottom
        }
    }

    Icon {
        anchors.centerIn: parent
        name: root.style.icon
        size: root.size * 0.5
        color: root.style.darkGlyph ? "#6b4a00" : "#ffffff"
    }
}
