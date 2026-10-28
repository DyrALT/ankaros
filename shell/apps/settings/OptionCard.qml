// Önizlemeli seçenek kartı (Görünüm ve Duvar kağıdı sayfaları). Önizleme çocuk olarak verilir.
import QtQuick

import Ankaros.UI

Rectangle {
    id: card

    property string label
    property bool selected
    default property alias preview: previewSlot.data

    signal chosen

    implicitWidth: column.implicitWidth + 20
    implicitHeight: column.implicitHeight + 22
    radius: 14
    color: selected ? Theme.color.accentSoft : Theme.color.surfaceHover
    border.color: selected ? Theme.color.accent : Theme.color.border
    border.width: selected ? 2 : 1

    Behavior on color {
        ColorAnimation { duration: Theme.motion.fast }
    }

    Column {
        id: column

        x: 10
        y: 10
        spacing: 10

        Item {
            id: previewSlot

            implicitWidth: childrenRect.width
            implicitHeight: childrenRect.height
            width: implicitWidth
            height: implicitHeight
        }

        Row {
            spacing: Theme.spacing.sm

            Radio {
                anchors.verticalCenter: parent.verticalCenter
                checked: card.selected
            }

            Label {
                anchors.verticalCenter: parent.verticalCenter
                text: card.label
                font.weight: card.selected ? Font.DemiBold : Font.Medium
            }
        }
    }

    TapHandler {
        onTapped: card.chosen()
    }
}
