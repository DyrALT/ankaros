// Ayarlar penceresinin içeriği: kenar çubuğu ve seçili sayfa. Kenar çubuğu her sayfada aynıdır.
// session: oturum işlemleri (Kullanıcı sayfası), network: ağ durumu ve ayarı (Ağ sayfası),
// bkz. Desktop.session ve Desktop.network.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Ankaros.UI

Rectangle {
    id: root

    property var session: null
    property var network: null
    property string page: "appearance"

    // Henüz yazılmamış sayfalar soluk görünür ve seçilemez
    readonly property var pages: [
        { id: "appearance", label: qsTr("Görünüm"), ready: true },
        { id: "wallpaper", label: qsTr("Duvar kağıdı"), ready: true },
        { id: "taskbar", label: qsTr("Alt çubuk"), ready: true },
        { id: "notifications", label: qsTr("Bildirimler"), ready: false },
        { id: "network", label: qsTr("Ağ"), ready: true },
        { id: "user", label: qsTr("Kullanıcı"), ready: true },
        { id: "keyboard", label: qsTr("Klavye ve dil"), ready: true },
        { id: "about", label: qsTr("Hakkında"), ready: true }
    ]

    color: Theme.color.surfaceWindow

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 200
            color: Theme.color.surfaceSidebar

            Column {
                anchors.fill: parent
                anchors.margins: 10
                anchors.topMargin: 8
                spacing: 2

                Repeater {
                    model: root.pages

                    Rectangle {
                        id: item

                        required property var modelData
                        readonly property bool current: root.page === modelData.id

                        width: parent.width
                        height: 34
                        radius: Theme.radius.sm
                        color: current ? Theme.color.accentSoft
                                       : hover.hovered && modelData.ready ? Theme.color.surfaceHover
                                                                          : "transparent"

                        Label {
                            anchors.verticalCenter: parent.verticalCenter
                            x: 10
                            text: item.modelData.label
                            color: item.current ? Theme.color.accent : Theme.color.textPrimary
                            font.weight: item.current ? Font.DemiBold : Font.Normal
                            opacity: item.modelData.ready ? 1 : 0.5
                        }

                        HoverHandler {
                            id: hover
                        }

                        TapHandler {
                            enabled: item.modelData.ready
                            onTapped: root.page = item.modelData.id
                        }
                    }
                }
            }
        }

        Loader {
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: ({
                    wallpaper: wallpaperPage,
                    taskbar: taskbarPage,
                    network: networkPage,
                    user: userPage,
                    keyboard: keyboardPage,
                    about: aboutPage
                })[root.page] ?? appearancePage
        }
    }

    Component {
        id: appearancePage

        AppearanceSettings {}
    }

    Component {
        id: wallpaperPage

        WallpaperSettings {}
    }

    Component {
        id: taskbarPage

        TaskbarSettings {}
    }

    Component {
        id: keyboardPage

        KeyboardSettings {}
    }

    Component {
        id: aboutPage

        AboutSettings {}
    }

    Component {
        id: networkPage

        NetworkSettings {
            network: root.network
        }
    }

    Component {
        id: userPage

        UserSettings {
            session: root.session
        }
    }
}
