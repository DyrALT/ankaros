// Üzerine gelince ve basılınca arka planı beliren düğme. Boyutu ve içeriği kullanan bileşen verir.
import QtQuick

Rectangle {
    id: root

    property bool checked: false
    property color restColor: "transparent"
    readonly property bool hovered: mouse.containsMouse
    readonly property bool pressed: mouse.pressed
    default property alias content: contentItem.data

    signal clicked

    radius: Theme.radius.md
    // Açıksa düğmeye Tab ile odaklanılır, Enter/Boşluk tıklar (ör. başlat paneli). Kapalıyken (varsayılan)
    // düğme Tab sırasına girmez ve tuşlar ona gitmez: başlık çubuğu düğmesine Enter'la basılıp pencere
    // kapanmasın.
    property bool keyboardNavigation: false

    activeFocusOnTab: keyboardNavigation
    border.width: root.visualFocus ? 2 : 0
    border.color: Theme.color.accent
    Keys.enabled: keyboardNavigation
    Keys.onReturnPressed: clicked()
    Keys.onEnterPressed: clicked()
    Keys.onSpacePressed: clicked()
    color: pressed ? Theme.color.surfacePressed
                   : hovered || checked ? Theme.color.surfaceHover : restColor

    Behavior on color {
        ColorAnimation { duration: Theme.motion.fast }
    }

    Item {
        id: contentItem

        anchors.fill: parent
        scale: root.pressed ? 0.94 : 1

        Behavior on scale {
            NumberAnimation { duration: Theme.motion.fast; easing.type: Theme.motion.easing }
        }
    }

    MouseArea {
        id: mouse

        anchors.fill: parent
        hoverEnabled: true
        onClicked: root.clicked()
    }
}
