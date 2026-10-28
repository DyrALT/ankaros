// Pencerenin ankaros'a ait kısmı: gölge, yuvarlak köşeli zemin, başlık çubuğu, taşıma ve boyutlandırma.
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
    // Başlık çubuğundan sürüklenerek taşınabilir mi
    property bool movable: true
    // Taşımayı pencere değil, move* sinyallerini dinleyen yapar (compositor: yaslama, büyütülmüş
    // pencereyi sürükleyince eski boyutuna döndürme). Kapalıyken pencere sürüklenen yere gider.
    property bool manualMove: false
    // Kenarlardan ve köşelerden boyutlandırılabilir mi. Boyutu resize* sinyallerini dinleyen değiştirir.
    property bool resizable: false
    // Sahnedeki bir noktada en üstteki pencere bu mu. Qt, üstteki pencere tıklamayı alsa bile alttaki
    // pencerelerin sürükleme ve dokunma tutucularına da olayı iletir. Verilmezse her zaman doğru sayılır.
    property var isTopAt: null
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
    // Konumlar sahne koordinatında (ekran)
    signal moveStarted(point position)
    signal moveUpdated(point position)
    signal moveFinished(point position)
    // edges: Qt.LeftEdge | Qt.TopEdge ... ; dx, dy: basılan yerden bu yana fare ne kadar kaydı
    signal resizeStarted(int edges)
    signal resizeUpdated(real dx, real dy)
    signal resizeFinished

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
            // Başka bir pencerenin altında kalan kısım tıklamaları almasın
            containmentMask: QtObject {
                function contains(point: point): bool {
                    if (point.x < 0 || point.y < 0 || point.x > titleBar.width || point.y > titleBar.height)
                        return false;
                    return !root.isTopAt || root.isTopAt(titleBar.mapToItem(null, point));
                }
            }

            DragHandler {
                id: moveHandler

                enabled: root.movable
                target: root.manualMove ? null : root
                cursorShape: Qt.ClosedHandCursor
                onActiveChanged: {
                    if (active)
                        root.moveStarted(centroid.scenePosition);
                    else
                        root.moveFinished(centroid.scenePosition);
                }
                onCentroidChanged: {
                    if (active)
                        root.moveUpdated(centroid.scenePosition);
                }
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

    // Boyutlandırma tutamakları: çerçevenin hemen dışında (gölgenin üstünde) ince şeritler ve köşeler.
    // İçeriğin üstüne taşmazlar, yoksa uygulamanın kenara yakın tıklamalarını yutarlar.
    Repeater {
        model: root.resizable ? [
            { edges: Qt.TopEdge | Qt.LeftEdge, cursor: Qt.SizeFDiagCursor },
            { edges: Qt.TopEdge | Qt.RightEdge, cursor: Qt.SizeBDiagCursor },
            { edges: Qt.BottomEdge | Qt.LeftEdge, cursor: Qt.SizeBDiagCursor },
            { edges: Qt.BottomEdge | Qt.RightEdge, cursor: Qt.SizeFDiagCursor },
            { edges: Qt.TopEdge, cursor: Qt.SizeVerCursor },
            { edges: Qt.BottomEdge, cursor: Qt.SizeVerCursor },
            { edges: Qt.LeftEdge, cursor: Qt.SizeHorCursor },
            { edges: Qt.RightEdge, cursor: Qt.SizeHorCursor }
        ] : []

        MouseArea {
            id: handle

            required property var modelData
            readonly property int edges: modelData.edges
            readonly property bool corner: (edges & (Qt.LeftEdge | Qt.RightEdge))
                                           && (edges & (Qt.TopEdge | Qt.BottomEdge))
            // Tutamağın dışarı taşan kalınlığı ve köşelerde içeri giren payı
            readonly property real outside: 6
            readonly property real cornerSize: 16
            property point pressPosition

            x: edges & Qt.LeftEdge ? -outside
               : edges & Qt.RightEdge ? root.width - (corner ? cornerSize - outside : 0)
               : cornerSize - outside
            y: edges & Qt.TopEdge ? -outside
               : edges & Qt.BottomEdge ? root.height - (corner ? cornerSize - outside : 0)
               : cornerSize - outside
            width: corner ? cornerSize
                   : edges & (Qt.LeftEdge | Qt.RightEdge) ? outside
                   : root.width - 2 * (cornerSize - outside)
            height: corner ? cornerSize
                    : edges & (Qt.TopEdge | Qt.BottomEdge) ? outside
                    : root.height - 2 * (cornerSize - outside)
            z: 10
            hoverEnabled: true
            preventStealing: true
            cursorShape: modelData.cursor

            onPressed: mouse => {
                pressPosition = mapToItem(null, mouse.x, mouse.y);
                root.pressed();
                root.resizeStarted(edges);
            }
            onPositionChanged: mouse => {
                if (!pressed)
                    return;
                const position = mapToItem(null, mouse.x, mouse.y);
                root.resizeUpdated(position.x - pressPosition.x, position.y - pressPosition.y);
            }
            onReleased: root.resizeFinished()
            onCanceled: root.resizeFinished()
        }
    }
}
