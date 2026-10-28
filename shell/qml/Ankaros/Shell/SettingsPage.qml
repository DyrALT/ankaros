// Ayarlar sayfalarının ortak düzeni: sayfa başlığı ve altında bölümler, kenar boşlukları Figma'daki gibi.
// Sayfada açıklama yazısı olmaz, açıklamalar SettingLabel'ın bilgi balonunda durur.
import QtQuick
import QtQuick.Layouts

import Ankaros.UI

Item {
    id: root

    property string title
    default property alias content: column.data

    ColumnLayout {
        id: column

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.leftMargin: 32
        anchors.rightMargin: 32
        anchors.topMargin: Theme.spacing.xl
        spacing: 22

        Label {
            text: root.title
            font.pixelSize: Theme.font.title
            font.weight: Font.Bold
        }
    }
}
