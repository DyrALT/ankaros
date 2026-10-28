// Giriş ve kilit ekranında şifre alanının altındaki hata mesajı
import QtQuick

import Ankaros.UI

Rectangle {
    id: root

    property alias text: label.text

    implicitWidth: label.implicitWidth + 20
    implicitHeight: label.implicitHeight + 8
    radius: 8
    color: Theme.color.windowClose
    opacity: text.length > 0 ? 1 : 0

    Behavior on opacity {
        NumberAnimation { duration: Theme.motion.fast }
    }

    Label {
        id: label

        anchors.centerIn: parent
        color: "#ffffff"
        font.pixelSize: Theme.font.small
        font.weight: Font.DemiBold
    }
}
