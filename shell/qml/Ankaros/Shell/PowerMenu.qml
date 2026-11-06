// Başlat panelindeki güç düğmesinin menüsü (Figma: Başlat paneli › Güç menüsü).
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Ankaros.UI

GlassPanel {
    id: root

    signal lockRequested
    signal logoutRequested
    signal rebootRequested
    signal poweroffRequested

    implicitWidth: 220
    implicitHeight: column.implicitHeight + 12
    radius: Theme.radius.md
    tint: Theme.color.surfaceWindow
    visible: opacity > 0

    Behavior on opacity {
        NumberAnimation { duration: Theme.motion.fast }
    }

    ColumnLayout {
        id: column

        anchors.fill: parent
        anchors.margins: 6
        spacing: 2

        MenuItem {
            icon: "lock"
            label: qsTr("Kilitle")
            shortcut: qsTr("Süper+L")
            onClicked: root.lockRequested()
        }

        MenuItem {
            icon: "logout"
            label: qsTr("Oturumu kapat")
            onClicked: root.logoutRequested()
        }

        MenuSeparator {}

        MenuItem {
            icon: "restart"
            label: qsTr("Yeniden başlat")
            onClicked: root.rebootRequested()
        }

        MenuItem {
            icon: "power"
            label: qsTr("Kapat")
            onClicked: root.poweroffRequested()
        }
    }
}
