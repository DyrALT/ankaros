// Ayar adının yanındaki bilgi simgesi. Üzerine gelince açıklama balonu açılır. Balon pencerenin en
// üst katmanına taşınır, böylece sayfadaki diğer öğelerin altında kalmaz ve kesilmez.
import QtQuick

Item {
    id: root

    property string text

    implicitWidth: 16
    implicitHeight: 16

    Icon {
        anchors.centerIn: parent
        name: "info"
        size: 14
        color: hover.hovered ? Theme.color.accent : Theme.color.textSecondary
    }

    HoverHandler {
        id: hover
    }

    Bubble {
        readonly property point position: hover.hovered && parent ? root.mapToItem(parent, 24, -10)
                                                                  : Qt.point(0, 0)

        parent: root.Window.contentItem
        x: position.x
        y: position.y
        text: root.text
        shown: hover.hovered
    }
}
