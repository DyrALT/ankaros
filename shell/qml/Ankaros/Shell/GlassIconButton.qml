// Giriş ekranındaki yuvarlak cam düğme. Adı yanına yazılmaz, üzerine gelince balonda görünür.
import QtQuick

Rectangle {
    id: root

    property string icon
    property string tooltip
    property string label  // simge yerine kısa bir metin (ör. klavye düzeni "TR")

    signal clicked

    implicitWidth: 44
    implicitHeight: 44
    radius: 22
    color: Qt.rgba(1, 1, 1, mouse.pressed ? 0.26 : mouse.containsMouse ? 0.22 : 0.16)
    border.color: Qt.rgba(1, 1, 1, 0.3)

    Icon {
        anchors.centerIn: parent
        visible: root.icon.length > 0
        name: root.icon
        size: 18
        color: "#ffffff"
    }

    Label {
        anchors.centerIn: parent
        visible: root.label.length > 0
        text: root.label
        color: "#ffffff"
        font.weight: Font.DemiBold
    }

    MouseArea {
        id: mouse

        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }

    Bubble {
        readonly property point position: mouse.containsMouse && parent
                                          ? root.mapToItem(parent, (root.width - width) / 2, -height - 8)
                                          : Qt.point(0, 0)

        parent: root.Window.contentItem
        x: position.x
        y: position.y
        text: root.tooltip
        shown: mouse.containsMouse && root.tooltip.length > 0
    }
}
