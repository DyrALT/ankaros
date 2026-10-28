// Ekranın altında yüzen tek çubuk: başlat düğmesi, arama, uygulamalar, durum simgeleri ve saat.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Ankaros.UI

GlassPanel {
    id: root

    // Çubukta gösterilecek uygulamalar (sabitlenmiş ve çalışanlar)
    property var apps: []
    property var running: []
    property string activeApp
    property bool startMenuOpen: false
    // Ağ durumu (Desktop.network). null ise ağ simgesi soluk görünür.
    property var network: null

    signal startClicked
    signal searchClicked
    signal appClicked(string app)
    signal networkClicked

    implicitHeight: Theme.size.taskBar
    radius: 18

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.spacing.sm
        anchors.rightMargin: Theme.spacing.lg
        spacing: Theme.spacing.sm

        HoverButton {
            Layout.preferredWidth: Theme.size.panelButton
            Layout.preferredHeight: Theme.size.panelButton
            restColor: Theme.color.accent
            color: pressed ? Qt.darker(Theme.color.accent, 1.15)
                           : hovered || root.startMenuOpen ? Qt.lighter(Theme.color.accent, 1.08) : restColor
            onClicked: root.startClicked()

            Icon {
                anchors.centerIn: parent
                name: "logo"
                size: 22
                color: Theme.color.textOnAccent
            }
        }

        HoverButton {
            Layout.preferredWidth: 220
            Layout.preferredHeight: 38
            restColor: Theme.color.surfaceHover
            onClicked: root.searchClicked()

            Row {
                anchors.verticalCenter: parent.verticalCenter
                x: Theme.spacing.md
                spacing: Theme.spacing.sm

                Icon {
                    anchors.verticalCenter: parent.verticalCenter
                    name: "search"
                    size: 15
                    color: Theme.color.textSecondary
                }

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Ara")
                    color: Theme.color.textSecondary
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: 1
            Layout.preferredHeight: 28
            color: Theme.color.border
        }

        Repeater {
            model: root.apps

            TaskItem {
                required property string modelData

                app: modelData
                running: root.running.includes(modelData)
                active: root.activeApp === modelData
                onClicked: root.appClicked(modelData)
            }
        }

        Item {
            Layout.fillWidth: true
        }

        // Durum simgeleri. Her simge kendi tıklamasını alır (ör. ağ simgesi Ayarlar › Ağ'ı açar), bu yüzden
        // grup HoverButton değil: onun fare alanı simgelerin üstünde kalırdı.
        Rectangle {
            Layout.preferredWidth: trayRow.implicitWidth + 2 * Theme.spacing.md
            Layout.preferredHeight: 34
            radius: Theme.radius.md
            color: trayHover.hovered ? Theme.color.surfaceHover : "transparent"

            Behavior on color {
                ColorAnimation { duration: Theme.motion.fast }
            }

            HoverHandler {
                id: trayHover
            }

            Row {
                id: trayRow

                anchors.centerIn: parent
                spacing: 14

                Icon {
                    name: "bell"
                    size: Theme.size.statusIcon
                }

                // Kablolu ağ: bağlı değilse soluk. Üzerine gelince durum, tıklayınca Ayarlar › Ağ.
                Icon {
                    id: networkIcon

                    readonly property bool connected: root.network !== null && root.network.connected

                    name: "network"
                    size: Theme.size.statusIcon
                    opacity: connected ? 1 : 0.4

                    HoverHandler {
                        id: networkHover
                    }

                    TapHandler {
                        onTapped: root.networkClicked()
                    }

                    Bubble {
                        readonly property point position: networkHover.hovered && parent
                            ? networkIcon.mapToItem(parent, (networkIcon.width - width) / 2, -height - 14)
                            : Qt.point(0, 0)

                        parent: networkIcon.Window.contentItem
                        x: position.x
                        y: position.y
                        shown: networkHover.hovered
                        text: !root.network || !root.network.available ? qsTr("Ağ servisine ulaşılamıyor")
                            : !root.network.carrier ? qsTr("Kablo takılı değil")
                            : root.network.connected ? qsTr("Kablolu · %1").arg(root.network.address.split("/")[0])
                            : qsTr("Bağlanıyor")
                    }
                }

                Icon {
                    name: "volume"
                    size: Theme.size.statusIcon
                }

                Icon {
                    name: "battery"
                    size: Theme.size.statusIcon
                }
            }
        }

        Clock {
            Layout.leftMargin: Theme.spacing.xs
        }
    }
}
