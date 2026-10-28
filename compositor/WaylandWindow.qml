// Bir uygulama penceresi: ankaros pencere çerçevesi (WindowChrome) içinde uygulamanın Wayland yüzeyi.
// Odak, öne getirme, küçültme, büyütme, yaslama, boyutlandırma ve animasyonlar burada yönetilir.
pragma ComponentBehavior: Bound

import QtQuick
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell

import Ankaros.Shell
import Ankaros.UI

WindowChrome {
    id: root

    required property XdgSurface xdgSurface
    // Pencerelerin kullanabileceği alan (alt çubuk hariç), büyütmede ve yaslamada kullanılır.
    property rect workArea
    // Uygulamanın kabuktaki adı ("Terminal" gibi), Main.qml app_id'ye göre verir.
    property string appName
    property bool minimized: false
    // Küçültülürken pencerenin uçacağı yer (alt çubuktaki simgesi). Main.qml verir.
    property var minimizeTarget: () => Qt.point(x + width / 2, workArea.y + workArea.height)
    // Ekranın yarısına yaslanmışsa "left" ya da "right"
    property string tiled: ""
    // Sürüklenirken bırakılırsa yaslanacağı yer: "left", "right", "top" (büyüt) ya da ""
    property string snapZone: ""
    readonly property rect snapRect: snapZone === "top" ? workArea : snapZone !== "" ? tileRect(snapZone)
                                                                                      : Qt.rect(0, 0, 0, 0)

    readonly property XdgToplevel toplevel: xdgSurface ? xdgSurface.toplevel : null
    readonly property string appId: toplevel ? toplevel.appId : ""
    readonly property bool maximized: toplevel ? toplevel.maximized : false

    // Büyütmeden ya da yaslamadan önceki konum ve boyut (içerik boyutu, çerçevesiz)
    property rect restoreGeometry
    // Pencere ilk boyutunu aldığında bir kez yerleştirilir
    property bool placed: false
    // Açılma ve kapanma animasyonu: 0 görünmez, 1 yerinde
    property real appear: 0
    property bool closing: false
    // Küçültme animasyonu: 0 yerinde, 1 alt çubukta
    property real minimizeProgress: minimized ? 1 : 0
    property point flyTarget

    // Boyutlandırma sürerken: tutulan kenarlar, başlangıç boyutu ve sabit kalan karşı kenar. Kenarlar
    // fare bırakıldıktan sonra da uygulama son boyutu gönderene kadar tutulur.
    property int resizeEdges: 0
    property bool resizing: false
    property size resizeStart
    property size resizeTarget
    property point resizeAnchor
    // Taşınırken farenin pencere içindeki yeri
    property point grabOffset

    signal raiseRequested
    signal gone

    // İlk boyut belli olunca pencereyi ekrana sığdır: kullanılabilir alandan büyükse büyüt,
    // değilse taşmayacak şekilde konumunu düzelt.
    function place() {
        if (placed || surfaceItem.width <= 0 || surfaceItem.height <= 0)
            return;
        placed = true;
        appear = 1;
        if (width > workArea.width || height > workArea.height) {
            maximize();
            // Eski boyutuna döndürülürse ekranın ortasında, ekrandan küçük bir pencere olsun
            restoreGeometry = Qt.rect(workArea.x + workArea.width * 0.15, workArea.y + workArea.height * 0.1,
                                      Math.round(workArea.width * 0.7), Math.round(workArea.height * 0.8));
            return;
        }
        x = Math.max(workArea.x, Math.min(x, workArea.x + workArea.width - width));
        y = Math.max(workArea.y, Math.min(y, workArea.y + workArea.height - height));
    }

    function finishResizeIfDone() {
        if (!resizing && surfaceItem.width === resizeTarget.width && surfaceItem.height === resizeTarget.height)
            resizeEdges = 0;
    }

    function takeFocus() {
        surfaceItem.takeFocus();
    }

    // Çerçeve dahil bir dikdörtgeni içerik boyutuna çevirir
    function contentSize(rect) {
        return Qt.size(Math.round(rect.width - 2 * frameWidth),
                       Math.round(rect.height - titleBarHeight - 2 * frameWidth));
    }

    function tileRect(side) {
        const half = Math.round(workArea.width / 2);
        return Qt.rect(side === "left" ? workArea.x : workArea.x + half, workArea.y,
                       side === "left" ? half : workArea.width - half, workArea.height);
    }

    // Serbest durumdaki konum ve boyutu sakla (büyütülmüş ya da yaslanmışsa eskisi kalır)
    function saveGeometry() {
        resizeEdges = 0;
        if (!maximized && tiled === "")
            restoreGeometry = Qt.rect(x, y, surfaceItem.width, surfaceItem.height);
    }

    function maximize() {
        saveGeometry();
        tiled = "";
        x = workArea.x;
        y = workArea.y;
        toplevel.sendMaximized(contentSize(workArea));
    }

    function tile(side) {
        saveGeometry();
        tiled = side;
        const rect = tileRect(side);
        x = rect.x;
        y = rect.y;
        toplevel.sendUnmaximized(contentSize(rect));
    }

    // Büyütülmüş ya da yaslanmış pencereyi eski konumuna ve boyutuna döndür
    function restore() {
        if (!maximized && tiled === "")
            return;
        resizeEdges = 0;
        tiled = "";
        x = restoreGeometry.x;
        y = restoreGeometry.y;
        toplevel.sendUnmaximized(Qt.size(restoreGeometry.width, restoreGeometry.height));
    }

    function toggleMaximize() {
        if (maximized)
            restore();
        else
            maximize();
    }

    function zoneAt(position) {
        const edge = 8;
        if (position.y <= workArea.y + edge)
            return "top";
        if (position.x <= workArea.x + edge)
            return "left";
        if (position.x >= workArea.x + workArea.width - edge)
            return "right";
        return "";
    }

    app: appName
    // Uygulama sunucu tarafı kenarlığı kabul etmediyse (kendi kenarlığını çizen uygulamalar) çerçeveyi o çizer.
    decorated: toplevel ? toplevel.decorationMode === XdgToplevel.ServerSideDecoration : true
    title: toplevel && toplevel.title.length > 0 ? toplevel.title : appName
    active: toplevel ? toplevel.activated : false
    manualMove: true
    resizable: decorated && !maximized && tiled === "" && !closing
    enabled: !closing
    visible: minimizeProgress < 1
    width: surfaceItem.width + 2 * frameWidth
    height: titleBarHeight + surfaceItem.height + 2 * frameWidth
    opacity: appear * (1 - minimizeProgress)
    transform: [
        Scale {
            readonly property real factor: (0.94 + 0.06 * root.appear) * (1 - 0.8 * root.minimizeProgress)

            origin.x: root.width / 2
            origin.y: root.height / 2
            xScale: factor
            yScale: factor
        },
        Translate {
            x: (root.flyTarget.x - root.x - root.width / 2) * root.minimizeProgress
            y: (root.flyTarget.y - root.y - root.height / 2) * root.minimizeProgress
        }
    ]

    Behavior on appear {
        NumberAnimation { duration: Theme.motion.normal; easing.type: Theme.motion.easing }
    }

    Behavior on minimizeProgress {
        NumberAnimation { duration: Theme.motion.normal; easing.type: Easing.InOutCubic }
    }

    onMinimizedChanged: {
        if (minimized)
            flyTarget = minimizeTarget();
    }
    onWidthChanged: {
        // Soldan boyutlandırırken sağ kenar yerinde kalsın (uygulama yeni boyutu sonradan gönderir)
        if (resizeEdges & Qt.LeftEdge)
            x = resizeAnchor.x - width;
        finishResizeIfDone();
        place();
    }
    onHeightChanged: {
        if (resizeEdges & Qt.TopEdge)
            y = resizeAnchor.y - height;
        finishResizeIfDone();
        place();
    }
    onPressed: {
        raiseRequested();
        takeFocus();
    }
    onCloseClicked: toplevel.sendClose()
    onMinimizeClicked: minimized = true
    onMaximizeClicked: toggleMaximize()

    onMoveStarted: position => {
        resizeEdges = 0;
        if (maximized || tiled !== "") {
            // Büyütülmüş ya da yaslanmış pencere sürüklenince eski boyutuna döner. Fare pencerenin
            // aynı oranındaki yerinde kalır.
            const ratio = (position.x - x) / width;
            const restoredWidth = restoreGeometry.width + 2 * frameWidth;
            grabOffset = Qt.point(ratio * restoredWidth, position.y - y);
            tiled = "";
            toplevel.sendUnmaximized(Qt.size(restoreGeometry.width, restoreGeometry.height));
        } else {
            grabOffset = Qt.point(position.x - x, position.y - y);
        }
    }
    onMoveUpdated: position => {
        x = position.x - grabOffset.x;
        // Başlık çubuğu ekranın üstünden çıkmasın, yoksa pencere bir daha tutulamaz
        y = Math.max(workArea.y, position.y - grabOffset.y);
        snapZone = zoneAt(position);
    }
    onMoveFinished: position => {
        const zone = zoneAt(position);
        snapZone = "";
        if (zone === "top")
            maximize();
        else if (zone !== "")
            tile(zone);
    }

    onResizeStarted: edges => {
        resizeEdges = edges;
        resizing = true;
        resizeStart = Qt.size(surfaceItem.width, surfaceItem.height);
        resizeTarget = resizeStart;
        resizeAnchor = Qt.point(x + width, y + height);
    }
    onResizeUpdated: (dx, dy) => {
        const min = toplevel.minSize;
        const max = toplevel.maxSize;
        let w = resizeStart.width + (resizeEdges & Qt.RightEdge ? dx : resizeEdges & Qt.LeftEdge ? -dx : 0);
        let h = resizeStart.height + (resizeEdges & Qt.BottomEdge ? dy : resizeEdges & Qt.TopEdge ? -dy : 0);
        w = Math.max(w, min.width > 0 ? min.width : 160);
        h = Math.max(h, min.height > 0 ? min.height : 100);
        if (max.width > 0)
            w = Math.min(w, max.width);
        if (max.height > 0)
            h = Math.min(h, max.height);
        const size = Qt.size(Math.round(w), Math.round(h));
        if (size.width !== resizeTarget.width || size.height !== resizeTarget.height) {
            resizeTarget = size;
            toplevel.sendResizing(size);
        }
    }
    onResizeFinished: {
        if (!resizing)
            return;
        resizing = false;
        toplevel.sendUnmaximized(resizeTarget);
        finishResizeIfDone();
    }

    ShellSurfaceItem {
        id: surfaceItem

        shellSurface: root.xdgSurface
        autoCreatePopupItems: true
        // Uygulama kendi sürükleme isteğini gönderirse (ör. kendi başlık alanından) pencereyi taşı
        moveItem: root
        onSurfaceDestroyed: {
            // Kapanma animasyonu için son kare ekranda kalsın
            bufferLocked = true;
            root.closing = true;
            root.appear = 0;
            root.gone();
            destroyTimer.start();
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
        id: destroyTimer

        interval: Theme.motion.normal
        onTriggered: root.destroy()
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
