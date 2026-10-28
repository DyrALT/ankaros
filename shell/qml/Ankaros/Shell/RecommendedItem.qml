// Başlat panelindeki son kullanılan öğe: simge, ad ve ne zaman kullanıldığı.
import QtQuick

import Ankaros.UI

HoverButton {
    id: root

    property string app
    property string title
    property string subtitle

    implicitHeight: 50
    radius: 10

    Row {
        anchors.verticalCenter: parent.verticalCenter
        x: Theme.spacing.sm
        spacing: Theme.spacing.md

        AppIcon {
            anchors.verticalCenter: parent.verticalCenter
            app: root.app
            size: 34
        }

        Column {
            anchors.verticalCenter: parent.verticalCenter
            spacing: 2

            Label {
                text: root.title
                font.weight: Font.Medium
            }

            Label {
                text: root.subtitle
                color: Theme.color.textSecondary
                font.pixelSize: Theme.font.caption
            }
        }
    }
}
