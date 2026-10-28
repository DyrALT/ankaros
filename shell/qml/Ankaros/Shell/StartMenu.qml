// Başlat paneli: arama, sabitlenmiş uygulamalar, önerilenler ve kullanıcı satırı.
// Alt çubuğun hemen üstünde, ortada açılır. Esc ya da panel dışına tıklama kapatır.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

GlassPanel {
    id: root

    property bool open: false
    property var apps: []
    // [{ app: "Metin Düzenleyici", title: "rapor.md", subtitle: "2 dakika önce" }, ...]
    property var recommended: []
    property string userName

    readonly property string query: search.text.trim().toLocaleLowerCase()
    readonly property var filteredApps: query.length === 0 ? apps
        : apps.filter(app => app.toLocaleLowerCase().includes(query))

    signal appLaunched(string app)
    signal lockRequested
    signal logoutRequested
    signal rebootRequested
    signal poweroffRequested

    function focusSearch() {
        search.forceActiveFocus();
    }

    implicitWidth: Theme.size.startMenuWidth
    implicitHeight: column.implicitHeight + 44
    radius: 24
    visible: opacity > 0
    opacity: open ? 1 : 0
    transform: Translate {
        y: root.open ? 0 : 16

        Behavior on y {
            NumberAnimation { duration: Theme.motion.normal; easing.type: Theme.motion.easing }
        }
    }

    Behavior on opacity {
        NumberAnimation { duration: Theme.motion.normal; easing.type: Theme.motion.easing }
    }

    onOpenChanged: {
        search.text = "";
        powerMenu.opacity = 0;
        if (open)
            search.forceActiveFocus();
    }

    Keys.onEscapePressed: root.open = false

    // Panelin içindeki tıklamalar arkadaki "dışarı tıklayınca kapat" alanına ulaşmasın.
    MouseArea {
        anchors.fill: parent
    }

    component SectionHeader: RowLayout {
        id: header

        property string title
        property string action

        Layout.fillWidth: true

        Label {
            Layout.fillWidth: true
            text: header.title
            font.weight: Font.DemiBold
        }

        HoverButton {
            Layout.preferredWidth: actionLabel.implicitWidth + 20
            Layout.preferredHeight: 24
            radius: Theme.radius.sm
            restColor: Theme.color.surfaceHover

            Label {
                id: actionLabel

                anchors.centerIn: parent
                text: header.action
                font.pixelSize: Theme.font.small
                font.weight: Font.Medium
            }
        }
    }

    ColumnLayout {
        id: column

        anchors.fill: parent
        anchors.margins: Theme.spacing.xl
        anchors.bottomMargin: 20
        spacing: 20

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            radius: Theme.radius.md
            color: Theme.color.surfaceHover
            border.color: Theme.color.accent
            border.width: 1.5

            Icon {
                id: searchIcon

                anchors.verticalCenter: parent.verticalCenter
                x: 14
                name: "search"
                size: 16
                color: Theme.color.textSecondary
            }

            TextInput {
                id: search

                anchors.left: searchIcon.right
                anchors.leftMargin: 10
                anchors.right: parent.right
                anchors.rightMargin: 14
                anchors.verticalCenter: parent.verticalCenter
                color: Theme.color.textPrimary
                selectionColor: Theme.color.accent
                font.family: Theme.font.family
                font.pixelSize: Theme.font.large
                clip: true
                Keys.onEscapePressed: root.open = false
                Keys.onReturnPressed: {
                    if (root.filteredApps.length > 0)
                        root.appLaunched(root.filteredApps[0]);
                }

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: search.text.length === 0
                    text: qsTr("Uygulama, dosya ya da ayar ara")
                    color: Theme.color.textSecondary
                    font.pixelSize: Theme.font.large
                }
            }
        }

        SectionHeader {
            title: root.query.length > 0 ? qsTr("Uygulamalar") : qsTr("Sabitlenmiş")
            action: qsTr("Tümü  ›")
        }

        Grid {
            Layout.fillWidth: true
            columns: 6
            rowSpacing: Theme.spacing.md

            Repeater {
                model: root.filteredApps

                AppTile {
                    required property string modelData

                    app: modelData
                    onClicked: root.appLaunched(modelData)
                }
            }
        }

        Label {
            visible: root.filteredApps.length === 0
            text: qsTr("“%1” ile eşleşen uygulama yok").arg(search.text)
            color: Theme.color.textSecondary
        }

        SectionHeader {
            visible: root.query.length === 0
            title: qsTr("Önerilenler")
            action: qsTr("Daha fazla  ›")
        }

        GridLayout {
            Layout.fillWidth: true
            visible: root.query.length === 0
            columns: 2
            columnSpacing: Theme.spacing.sm
            rowSpacing: Theme.spacing.xs

            Repeater {
                model: root.recommended

                RecommendedItem {
                    required property var modelData

                    Layout.fillWidth: true
                    app: modelData.app
                    title: modelData.title
                    subtitle: modelData.subtitle
                }
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.color.border
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Avatar {
                name: root.userName
                size: 32
            }

            Label {
                Layout.fillWidth: true
                text: root.userName
                font.weight: Font.Medium
            }

            HoverButton {
                Layout.preferredWidth: 36
                Layout.preferredHeight: 36
                radius: Theme.radius.sm
                onClicked: root.appLaunched("Ayarlar")

                AppIcon {
                    anchors.centerIn: parent
                    app: "Ayarlar"
                    size: 28
                }
            }

            HoverButton {
                id: powerButton

                Layout.preferredWidth: 36
                Layout.preferredHeight: 36
                radius: Theme.radius.sm
                checked: powerMenu.opacity > 0
                onClicked: powerMenu.opacity = powerMenu.opacity > 0 ? 0 : 1

                Icon {
                    anchors.centerIn: parent
                    name: "power"
                    size: 18
                }
            }
        }
    }

    PowerMenu {
        id: powerMenu

        x: parent.width - width - 22
        y: parent.height - height - 64
        opacity: 0
        onLockRequested: {
            root.open = false;
            root.lockRequested();
        }
        onLogoutRequested: root.logoutRequested()
        onRebootRequested: root.rebootRequested()
        onPoweroffRequested: root.poweroffRequested()
    }
}
