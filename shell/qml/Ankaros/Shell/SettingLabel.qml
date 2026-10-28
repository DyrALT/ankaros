// Ayar adı ve isteğe bağlı bilgi simgesi. Açıklama sayfaya yazılmaz, info verilirse balonda görünür.
import QtQuick

Row {
    id: root

    property alias text: label.text
    property string info
    property alias font: label.font

    spacing: 6

    Label {
        id: label

        anchors.verticalCenter: parent.verticalCenter
        font.weight: Font.Medium
    }

    InfoTip {
        anchors.verticalCenter: parent.verticalCenter
        visible: root.info.length > 0
        text: root.info
    }
}
