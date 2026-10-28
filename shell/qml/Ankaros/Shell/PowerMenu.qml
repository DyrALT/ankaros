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

    component MenuItem: HoverButton {
        id: item

        property string icon
        property string label
        property string shortcut

        Layout.fillWidth: true
        Layout.preferredHeight: 34
        radius: Theme.radius.sm

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 10

            Icon {
                name: item.icon
                size: 16
            }

            Label {
                Layout.fillWidth: true
                text: item.label
                font.weight: Font.Medium
            }

            Label {
                visible: item.shortcut.length > 0
                text: item.shortcut
                color: Theme.color.textSecondary
                font.pixelSize: Theme.font.caption
            }
        }
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

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            Layout.topMargin: 2
            Layout.bottomMargin: 2
            color: Theme.color.border
        }

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
