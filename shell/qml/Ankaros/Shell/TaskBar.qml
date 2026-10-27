// Ekranın altında yüzen tek çubuk: başlat düğmesi, arama, uygulamalar, durum simgeleri ve saat.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

GlassPanel {
    id: root

    // Çubukta gösterilecek uygulamalar (sabitlenmiş ve çalışanlar)
    property var apps: []
    property var running: []
    property string activeApp
    property bool startMenuOpen: false

    signal startClicked
    signal searchClicked
    signal appClicked(string app)

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

        HoverButton {
            Layout.preferredWidth: trayRow.implicitWidth + 2 * Theme.spacing.md
            Layout.preferredHeight: 34

            Row {
                id: trayRow

                anchors.centerIn: parent
                spacing: 14

                Repeater {
                    model: ["bell", "wifi", "volume", "battery"]

                    Icon {
                        required property string modelData

                        name: modelData
                        size: Theme.size.statusIcon
                    }
                }
            }
        }

        Clock {
            Layout.leftMargin: Theme.spacing.xs
        }
    }
}
