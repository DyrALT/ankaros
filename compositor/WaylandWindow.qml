// Bir uygulama penceresi: ankaros pencere çerçevesi (WindowChrome) içinde uygulamanın Wayland yüzeyi.
// Odak, öne getirme, küçültme, büyütme ve kapatma burada yönetilir.
pragma ComponentBehavior: Bound

import QtQuick
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell

import Ankaros.Shell

WindowChrome {
    id: root

    required property XdgSurface xdgSurface
    // Pencerelerin kullanabileceği alan (alt çubuk hariç), büyütmede kullanılır.
    property rect workArea
    // Uygulamanın kabuktaki adı ("Terminal" gibi), Main.qml app_id'ye göre verir.
    property string appName
    property bool minimized: false

    readonly property XdgToplevel toplevel: xdgSurface.toplevel
    readonly property string appId: toplevel ? toplevel.appId : ""
    readonly property bool maximized: toplevel ? toplevel.maximized : false

    // Büyütmeden önceki konum ve boyut
    property rect restoreGeometry
    // Pencere ilk boyutunu aldığında bir kez yerleştirilir
    property bool placed: false

    signal raiseRequested
    signal gone

    // İlk boyut belli olunca pencereyi ekrana sığdır: kullanılabilir alandan büyükse büyüt,
    // değilse taşmayacak şekilde konumunu düzelt.
    function place() {
        if (placed || surfaceItem.width <= 0 || surfaceItem.height <= 0)
            return;
        placed = true;
        if (width > workArea.width || height > workArea.height) {
            toggleMaximize();
            return;
        }
        x = Math.max(workArea.x, Math.min(x, workArea.x + workArea.width - width));
        y = Math.max(workArea.y, Math.min(y, workArea.y + workArea.height - height));
    }

    function takeFocus() {
        surfaceItem.takeFocus();
    }

    function toggleMaximize() {
        if (maximized) {
            x = restoreGeometry.x;
            y = restoreGeometry.y;
            toplevel.sendUnmaximized(Qt.size(restoreGeometry.width, restoreGeometry.height));
        } else {
            restoreGeometry = Qt.rect(x, y, surfaceItem.width, surfaceItem.height);
            x = workArea.x;
            y = workArea.y;
            toplevel.sendMaximized(Qt.size(workArea.width - 2 * frameWidth,
                                            workArea.height - titleBarHeight - 2 * frameWidth));
        }
    }

    app: appName
    // Uygulama sunucu tarafı kenarlığı kabul etmediyse (kendi kenarlığını çizen uygulamalar) çerçeveyi o çizer.
    decorated: toplevel ? toplevel.decorationMode === XdgToplevel.ServerSideDecoration : true
    title: toplevel && toplevel.title.length > 0 ? toplevel.title : appName
    active: toplevel ? toplevel.activated : false
    movable: !maximized
    visible: !minimized
    width: surfaceItem.width + 2 * frameWidth
    height: titleBarHeight + surfaceItem.height + 2 * frameWidth

    onWidthChanged: place()
    onHeightChanged: place()
    onPressed: {
        raiseRequested();
        takeFocus();
    }
    onCloseClicked: toplevel.sendClose()
    onMinimizeClicked: minimized = true
    onMaximizeClicked: toggleMaximize()

    ShellSurfaceItem {
        id: surfaceItem

        shellSurface: root.xdgSurface
        autoCreatePopupItems: true
        // Uygulama kendi sürükleme isteğini gönderirse (ör. kendi başlık alanından) pencereyi taşı
        moveItem: root
        onSurfaceDestroyed: {
            root.gone();
            root.destroy();
        }
    }

    // Uygulamaya "kare çizildi" bildirimi (frame callback). Qt bunu compositor'ın ekran karesinden
    // sonra gönderiyor, ama uygulamanın yeni karesi ekran çizilirken gelirse bildirim bir sonraki ekran
    // karesine kalıyor. Qt uygulamaları bildirimi 100 ms içinde alamazsa pencereyi gizli sayıp çizmeyi
    // bırakıyor, ekran bir sonraki girdi olayına kadar eski kalıyordu (yazılımsal çizimde, ör. QEMU).
    // Bu yüzden uygulama yeni kare gönderdikten kısa süre sonra bildirim ayrıca gönderilir.
    Connections {
        target: surfaceItem.surface

        function onRedraw() {
            frameCallbackTimer.restart();
        }
    }

    Timer {
        id: frameCallbackTimer

        interval: 16
        onTriggered: {
            if (surfaceItem.surface) {
                surfaceItem.surface.frameStarted();
                surfaceItem.surface.sendFrameCallbacks();
            }
        }
    }
}
