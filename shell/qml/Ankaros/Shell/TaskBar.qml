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
    // Pil (Desktop.battery). Pil yoksa simgesi görünmez.
    property var battery: null
    property bool quickSettingsOpen: false
    property bool calendarOpen: false

    signal startClicked
    signal searchClicked
    signal appClicked(string app)
    // Durum simgelerine tıklandı (hızlı ayarlar)
    signal statusClicked
    signal clockClicked

    // Uygulamanın simgesinin ortası, sahne koordinatında. Çubukta yoksa çubuğun ortası.
    function iconCenter(app) {
        for (let i = 0; i < taskItems.count; ++i) {
            const item = taskItems.itemAt(i) as TaskItem;
            if (item && item.app === app)
                return item.mapToItem(null, item.width / 2, item.height / 2);
        }
        return mapToItem(null, width / 2, height / 2);
    }

    implicitHeight: Theme.size.taskBar
    radius: 18

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.spacing.sm
        anchors.rightMargin: Theme.spacing.sm
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
            id: taskItems

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

        // Durum simgeleri: tıklayınca hızlı ayarlar açılır. Simgelerin üzerine gelince ayrıntı balonu
        // çıkar, bu yüzden grup HoverButton değil: onun fare alanı simgelerin üstünde kalırdı.
        Rectangle {
            Layout.preferredWidth: trayRow.implicitWidth + 2 * Theme.spacing.md
            Layout.preferredHeight: 34
            radius: Theme.radius.md
            color: trayTap.pressed ? Theme.color.surfacePressed
                 : trayHover.hovered || root.quickSettingsOpen ? Theme.color.surfaceHover : "transparent"

            Behavior on color {
                ColorAnimation { duration: Theme.motion.fast }
            }

            HoverHandler {
                id: trayHover
            }

            TapHandler {
                id: trayTap

                onTapped: root.statusClicked()
            }

            Row {
                id: trayRow

                anchors.centerIn: parent
                spacing: 14

                // Kablolu ağ: bağlı değilse soluk. Üzerine gelince durumu gösterir.
                Icon {
                    id: networkIcon

                    readonly property bool connected: root.network !== null && root.network.connected

                    name: "network"
                    size: Theme.size.statusIcon
                    opacity: connected ? 1 : 0.4

                    HoverHandler {
                        id: networkHover
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
                    id: batteryIcon

                    visible: root.battery !== null && root.battery.present
                    name: "battery"
                    size: Theme.size.statusIcon

                    HoverHandler {
                        id: batteryHover
                    }

                    Bubble {
                        readonly property point position: batteryHover.hovered && parent
                            ? batteryIcon.mapToItem(parent, (batteryIcon.width - width) / 2, -height - 14)
                            : Qt.point(0, 0)

                        parent: batteryIcon.Window.contentItem
                        x: position.x
                        y: position.y
                        shown: batteryHover.hovered
                        text: !root.battery ? "" : root.battery.charging
                            ? qsTr("%%1 · şarj oluyor").arg(root.battery.level) : qsTr("%%1").arg(root.battery.level)
                    }
                }
            }
        }

        HoverButton {
            Layout.preferredWidth: clock.implicitWidth + 2 * Theme.spacing.sm
            Layout.preferredHeight: 38
            radius: Theme.radius.md
            checked: root.calendarOpen
            onClicked: root.clockClicked()

            Clock {
                id: clock

                anchors.centerIn: parent
            }
        }
    }
}
