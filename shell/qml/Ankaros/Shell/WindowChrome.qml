// Pencerenin ankaros'a ait kısmı: gölge, yuvarlak köşeli zemin, başlık çubuğu ve taşıma.
// İçerik (uygulamanın kendisi) çocuk olarak verilir: compositor'da Wayland yüzeyi, önizlemede
// sahte bir içerik. Wayland türlerine bağımlı değildir.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Effects

Item {
    id: root

    property string app
    property string title
    property bool active: true
    // Başlık çubuğundan sürüklenerek taşınabilir mi (büyütülmüş pencerelerde kapalı)
    property bool movable: true
    readonly property real titleBarHeight: titleBar.height
    default property alias content: contentArea.data

    signal pressed
    signal minimizeClicked
    signal maximizeClicked
    signal closeClicked

    RectangularShadow {
        anchors.fill: parent
        radius: Theme.radius.md
        offset.y: root.active ? 20 : 10
        blur: root.active ? 56 : 28
        color: Qt.rgba(0, 0, 0, root.active ? 0.24 : 0.14)

        Behavior on blur {
            NumberAnimation { duration: Theme.motion.normal }
        }
    }

    Rectangle {
        id: background

        anchors.fill: parent
        radius: Theme.radius.md
        color: Theme.color.surfaceWindow
        border.color: Theme.color.border
        border.width: 1
    }

    // İçerik köşelerden taşmasın diye pencere yuvarlak köşeli bir maskeyle kırpılır.
    Item {
        id: body

        anchors.fill: parent
        anchors.margins: 1
        layer.enabled: true
        layer.effect: MultiEffect {
            maskEnabled: true
            maskSource: mask
        }

        WindowTitleBar {
            id: titleBar

            anchors.left: parent.left
            anchors.right: parent.right
            app: root.app
            title: root.title
            active: root.active
            onMinimizeClicked: root.minimizeClicked()
            onMaximizeClicked: root.maximizeClicked()
            onCloseClicked: root.closeClicked()

            DragHandler {
                enabled: root.movable
                target: root
                cursorShape: Qt.ClosedHandCursor
            }

            TapHandler {
                // Başlık çubuğuna çift tıklamak pencereyi büyütür ya da eski boyutuna döndürür.
                onDoubleTapped: root.maximizeClicked()
            }
        }

        Item {
            id: contentArea

            anchors.top: titleBar.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
        }
    }

    Item {
        id: mask

        anchors.fill: body
        visible: false
        layer.enabled: true

        Rectangle {
            anchors.fill: parent
            radius: Theme.radius.md - 1
        }
    }

    // Pencerenin herhangi bir yerine basılınca öne gelsin. Olayı tüketmez, içerik tıklamayı alır.
    TapHandler {
        gesturePolicy: TapHandler.DragThreshold
        grabPermissions: PointerHandler.ApprovesTakeOverByAnything
        onPressedChanged: {
            if (pressed)
                root.pressed();
        }
    }
}
