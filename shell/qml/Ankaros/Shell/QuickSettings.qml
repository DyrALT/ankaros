// Alt çubuktaki durum simgelerine tıklanınca açılan hızlı ayarlar (Figma: Hızlı ayarlar): ağ ve koyu tema
// kutucukları, kullanıcı, kilitleme, Ayarlar ve güç düğmeleri. Yalnızca gerçekten çalışan ayarlar
// konur, ses ve bildirimler Faz 4'te eklenecek.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Ankaros.UI

GlassPanel {
    id: root

    property bool open: false
    property string userName
    // Desktop.network (null ise ağ bilinmiyor)
    property var network: null

    signal networkClicked
    signal settingsClicked
    signal lockRequested
    signal logoutRequested
    signal rebootRequested
    signal poweroffRequested

    implicitWidth: 380
    implicitHeight: column.implicitHeight + 2 * Theme.spacing.lg
    radius: Theme.radius.lg
    visible: opacity > 0
    opacity: open ? 1 : 0
    transform: Translate {
        y: root.open ? 0 : 12

        Behavior on y {
            NumberAnimation { duration: Theme.motion.normal; easing.type: Theme.motion.easing }
        }
    }

    Behavior on opacity {
        NumberAnimation { duration: Theme.motion.normal; easing.type: Theme.motion.easing }
    }

    onOpenChanged: powerMenu.opacity = 0

    // Paneldeki tıklamalar arkadaki "dışarı tıklayınca kapat" alanına ulaşmasın
    MouseArea {
        anchors.fill: parent
    }

    // Açık ya da kapalı bir ayarı gösteren kutucuk. Açıkken vurgu renginde.
    component Tile: Rectangle {
        id: tile

        property string icon
        property string title
        property string subtitle
        property bool on: false
        // Ayrıntı sayfası varsa sağda ok
        property bool more: false

        signal clicked

        Layout.fillWidth: true
        Layout.preferredHeight: 56
        radius: Theme.radius.md
        color: on ? (tileMouse.pressed ? Qt.darker(Theme.color.accent, 1.1) : Theme.color.accent)
                  : tileMouse.pressed ? Theme.color.surfacePressed
                  : tileMouse.containsMouse ? Theme.color.surfaceHover : Qt.alpha(Theme.color.surfaceHover, 0.6)

        Behavior on color {
            ColorAnimation { duration: Theme.motion.fast }
        }

        MouseArea {
            id: tileMouse

            anchors.fill: parent
            hoverEnabled: true
            onClicked: tile.clicked()
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 10
            anchors.rightMargin: 10
            spacing: 10

            Rectangle {
                Layout.preferredWidth: 32
                Layout.preferredHeight: 32
                radius: 16
                color: tile.on ? Qt.rgba(1, 1, 1, 0.2) : Theme.color.surfaceHover

                Icon {
                    anchors.centerIn: parent
                    name: tile.icon
                    size: 16
                    color: tile.on ? Theme.color.textOnAccent : Theme.color.textPrimary
                }
            }

            Column {
                Layout.fillWidth: true
                spacing: 1

                Label {
                    width: parent.width
                    text: tile.title
                    color: tile.on ? Theme.color.textOnAccent : Theme.color.textPrimary
                    font.weight: Font.DemiBold
                    elide: Text.ElideRight
                }

                Label {
                    width: parent.width
                    text: tile.subtitle
                    color: tile.on ? Qt.alpha(Theme.color.textOnAccent, 0.85) : Theme.color.textSecondary
                    font.pixelSize: Theme.font.caption
                    elide: Text.ElideRight
                }
            }

            Icon {
                visible: tile.more
                name: "chevronRight"
                size: 14
                color: tile.on ? Theme.color.textOnAccent : Theme.color.textSecondary
            }
        }
    }

    component RoundButton: HoverButton {
        Layout.preferredWidth: 34
        Layout.preferredHeight: 34
        radius: 17
        restColor: Theme.color.surfaceHover
    }

    ColumnLayout {
        id: column

        x: Theme.spacing.lg
        y: Theme.spacing.lg
        width: root.width - 2 * Theme.spacing.lg
        spacing: 14

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Tile {
                readonly property bool connected: root.network !== null && root.network.connected

                icon: "network"
                title: qsTr("Ağ")
                subtitle: !root.network || !root.network.available ? qsTr("Ulaşılamıyor")
                    : !root.network.carrier ? qsTr("Kablo takılı değil")
                    : connected ? qsTr("Kablolu · bağlı") : qsTr("Bağlanıyor")
                on: connected
                more: true
                onClicked: root.networkClicked()
            }

            Tile {
                icon: "moon"
                title: qsTr("Koyu tema")
                subtitle: Appearance.darkMode ? qsTr("Açık") : qsTr("Kapalı")
                on: Appearance.darkMode
                onClicked: Appearance.darkMode = !Appearance.darkMode
            }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            Avatar {
                name: root.userName
                size: 28
            }

            Label {
                Layout.fillWidth: true
                text: root.userName
                font.weight: Font.DemiBold
                elide: Text.ElideRight
            }

            RoundButton {
                onClicked: root.lockRequested()

                Icon {
                    anchors.centerIn: parent
                    name: "lock"
                    size: 16
                }
            }

            RoundButton {
                onClicked: root.settingsClicked()

                AppIcon {
                    anchors.centerIn: parent
                    app: "Ayarlar"
                    size: 22
                }
            }

            RoundButton {
                checked: powerMenu.opacity > 0
                onClicked: powerMenu.opacity = powerMenu.opacity > 0 ? 0 : 1

                Icon {
                    anchors.centerIn: parent
                    name: "power"
                    size: 16
                }
            }
        }
    }

    PowerMenu {
        id: powerMenu

        x: parent.width - width
        y: -height - Theme.spacing.sm
        opacity: 0
        onLockRequested: root.lockRequested()
        onLogoutRequested: root.logoutRequested()
        onRebootRequested: root.rebootRequested()
        onPoweroffRequested: root.poweroffRequested()
    }
}
