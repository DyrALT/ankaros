// Menü satırı: simge, etiket ve (varsa) kısayol. Güç menüsü ve masaüstü menüsü kullanır.
// Bir ColumnLayout içinde durur, genişliği doldurur.
import QtQuick
import QtQuick.Layouts

HoverButton {
    id: root

    property string icon
    property string label
    property string shortcut

    Layout.fillWidth: true
    Layout.preferredHeight: 34
    radius: Theme.radius.sm

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 10

        Icon {
            name: root.icon
            size: 16
        }

        Label {
            Layout.fillWidth: true
            text: root.label
            font.weight: Font.Medium
        }

        Label {
            visible: root.shortcut.length > 0
            text: root.shortcut
            color: Theme.color.textSecondary
            font.pixelSize: Theme.font.caption
        }
    }
}
