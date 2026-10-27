// Ekranın altında yüzen panel: uygulama başlatıcı düğmesi ve saat.
import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    property bool launcherOpen: false

    signal launcherToggled

    border.color: Theme.color.border
    border.width: 1
    color: Theme.color.surfacePanel
    implicitHeight: Theme.size.panelHeight
    implicitWidth: row.implicitWidth + 2 * Theme.spacing.sm
    radius: Theme.radius.lg

    RowLayout {
        id: row

        anchors.centerIn: parent
        spacing: Theme.spacing.sm

        PanelButton {
            checked: root.launcherOpen

            onClicked: root.launcherToggled()

            LauncherIcon {
                anchors.centerIn: parent
            }
        }
        Rectangle {
            Layout.preferredHeight: Theme.size.panelButton - Theme.spacing.md
            Layout.preferredWidth: 1
            color: Theme.color.border
        }
        Clock {
            Layout.leftMargin: Theme.spacing.sm
            Layout.rightMargin: Theme.spacing.md
        }
    }
}
