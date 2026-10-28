// Pencerenin ankaros'a ait kısmı: gölge, yuvarlak köşeli zemin, başlık çubuğu ve taşıma.
// İçerik (uygulamanın kendisi) çocuk olarak verilir: compositor'da Wayland yüzeyi, önizlemede
// sahte bir içerik. Wayland türlerine bağımlı değildir.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Effects

import Ankaros.UI

Item {
    id: root

    property string app
    property string title
    property bool active: true
    // Başlık çubuğundan sürüklenerek taşınabilir mi (büyütülmüş pencerelerde kapalı)
    property bool movable: true
    // Çerçeveyi (gölge, zemin, başlık çubuğu, yuvarlak köşeler) çiz. Kendi kenarlığını çizmekte
    // ısrar eden uygulamalarda kapatılır, yoksa çift başlık çubuğu görünür (karar 0007).
    property bool decorated: true
    readonly property real titleBarHeight: decorated ? titleBar.height : 0
    // Çerçevenin içerik çevresindeki kalınlığı (1 px kenarlık)
    readonly property real frameWidth: decorated ? 1 : 0
    default property alias content: contentArea.data

    signal pressed
    signal minimizeClicked
    signal maximizeClicked
    signal closeClicked

    RectangularShadow {
        visible: root.decorated
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

        visible: root.decorated
        anchors.fill: parent
        radius: Theme.radius.md
        color: Theme.color.surfaceWindow
        border.color: Theme.color.border
        border.width: 1
    }

    Item {
        id: body

        anchors.fill: parent
        anchors.margins: root.frameWidth

        WindowTitleBar {
            id: titleBar

            visible: root.decorated
            anchors.left: parent.left
            anchors.right: parent.right
            topLeftRadius: Theme.radius.md - 1
            topRightRadius: Theme.radius.md - 1
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

        // İçerik alt köşelerden taşmasın diye katman olarak yuvarlak köşeli bir maskeyle çizilir.
        Item {
            id: contentArea

            anchors.top: root.decorated ? titleBar.bottom : parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            layer.enabled: root.decorated
            layer.effect: MultiEffect {
                maskEnabled: true
                maskSource: mask
            }
        }

        Item {
            id: mask

            anchors.fill: contentArea
            visible: false
            layer.enabled: true

            Rectangle {
                anchors.fill: parent
                bottomLeftRadius: Theme.radius.md - 1
                bottomRightRadius: Theme.radius.md - 1
            }
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
