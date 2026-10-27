// Başlat panelindeki sabitlenmiş uygulama: simge ve kısa ad.
import QtQuick

import "IconData.js" as IconData

HoverButton {
    id: root

    property string app

    implicitWidth: Theme.size.pinnedTile
    implicitHeight: Theme.size.pinnedIcon + 40

    Column {
        anchors.centerIn: parent
        spacing: Theme.spacing.sm

        AppIcon {
            anchors.horizontalCenter: parent.horizontalCenter
            app: root.app
            size: Theme.size.pinnedIcon
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            width: root.width - Theme.spacing.sm
            horizontalAlignment: Text.AlignHCenter
            elide: Text.ElideRight
            text: (IconData.apps[root.app] ?? IconData.fallbackApp).shortName || root.app
            font.pixelSize: Theme.font.small
            font.weight: Font.Medium
        }
    }
}
