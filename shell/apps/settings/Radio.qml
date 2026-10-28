// Seçim dairesi: seçenek kartlarında hangisinin seçili olduğunu gösterir.
import QtQuick

import Ankaros.UI

Rectangle {
    id: radio

    property bool checked

    implicitWidth: 16
    implicitHeight: 16
    radius: 8
    color: radio.checked ? "#ffffff" : "transparent"
    border.color: radio.checked ? Theme.color.accent : Qt.alpha(Theme.color.textSecondary, 0.6)
    border.width: radio.checked ? 5 : 1.5
}
