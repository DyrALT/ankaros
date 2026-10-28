// Pencere başlık çubuğu: uygulama simgesi, başlık ve küçült/büyüt/kapat düğmeleri.
// Compositor tarafından bütün uygulamalar için çizilir (karar 0007). Figma: WindowTitleBar.
// Düğme konumu ve stili Faz 1 C'de Görünüm ayarlarından gelecek, şimdilik sağda ve simgeli.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    property string app
    property string title
    property bool active: true

    signal minimizeClicked
    signal maximizeClicked
    signal closeClicked

    implicitHeight: 46
    color: Theme.color.surfaceWindow

    component WindowButton: HoverButton {
        id: button

        property string icon

        Layout.preferredWidth: 28
        Layout.preferredHeight: 28
        radius: 14

        Icon {
            anchors.centerIn: parent
            name: button.icon
            size: 14
            color: root.active ? Theme.color.textPrimary : Theme.color.textSecondary
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.spacing.lg
        anchors.rightMargin: Theme.spacing.md
        spacing: 10

        AppIcon {
            app: root.app
            size: 20
        }

        Label {
            Layout.fillWidth: true
            text: root.title
            elide: Text.ElideRight
            color: root.active ? Theme.color.textPrimary : Theme.color.textSecondary
            font.weight: Font.DemiBold
        }

        WindowButton {
            icon: "minimize"
            onClicked: root.minimizeClicked()
        }

        WindowButton {
            icon: "maximize"
            onClicked: root.maximizeClicked()
        }

        WindowButton {
            icon: "close"
            restColor: Theme.color.surfaceHover
            onClicked: root.closeClicked()
        }
    }
}
