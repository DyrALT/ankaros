// Tam ekran uygulama başlatıcı: arama kutusu ve uygulama ızgarası.
// Esc ya da boş alana tıklama kapatır.
pragma ComponentBehavior: Bound

import QtQuick

Item {
    id: root

    property var apps: []
    readonly property var filteredApps: apps.filter(app => app.name.toLocaleLowerCase().includes(
                                                               search.text.toLocaleLowerCase()))
    property bool open: false

    opacity: open ? 1 : 0
    visible: opacity > 0

    Behavior on opacity {
        NumberAnimation {
            duration: Theme.motion.normal
            easing.type: Theme.motion.easing
        }
    }

    Keys.onEscapePressed: root.open = false
    onOpenChanged: {
        search.text = "";
        if (open)
            search.forceActiveFocus();
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.color.scrim

        MouseArea {
            anchors.fill: parent

            onClicked: root.open = false
        }
    }
    Column {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.top: parent.top
        anchors.topMargin: parent.height * 0.12
        scale: root.open ? 1 : 0.96
        spacing: Theme.spacing.xl
        width: Math.min(parent.width - 2 * Theme.spacing.xxl, 6 * Theme.size.appTile)

        Behavior on scale {
            NumberAnimation {
                duration: Theme.motion.normal
                easing.type: Theme.motion.easing
            }
        }

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            border.color: Theme.color.border
            color: Theme.color.surfacePanel
            height: 44
            radius: Theme.radius.md
            width: Math.min(parent.width, 480)

            TextInput {
                id: search

                anchors.fill: parent
                anchors.leftMargin: Theme.spacing.lg
                anchors.rightMargin: Theme.spacing.lg
                clip: true
                color: Theme.color.textPrimary
                font.pixelSize: Theme.font.body
                verticalAlignment: TextInput.AlignVCenter

                Keys.onEscapePressed: root.open = false

                Text {
                    anchors.verticalCenter: parent.verticalCenter
                    color: Theme.color.textSecondary
                    font.pixelSize: Theme.font.body
                    text: qsTr("Uygulama ara")
                    visible: search.text.length === 0
                }
            }
        }
        GridView {
            cellHeight: Theme.size.appTile
            cellWidth: Theme.size.appTile
            height: Math.ceil(count / Math.max(1, Math.floor(width / cellWidth))) * cellHeight
            interactive: false
            model: root.filteredApps
            width: parent.width

            delegate: AppTile {
                required property var modelData

                name: modelData.name
                tint: modelData.color

                onClicked: root.open = false
            }
        }
    }
}
