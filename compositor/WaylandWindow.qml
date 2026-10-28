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

    signal raiseRequested
    signal gone

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
            toplevel.sendMaximized(Qt.size(workArea.width - 2, workArea.height - titleBarHeight - 2));
        }
    }

    app: appName
    title: toplevel && toplevel.title.length > 0 ? toplevel.title : appName
    active: toplevel ? toplevel.activated : false
    movable: !maximized
    visible: !minimized
    // Çerçevenin 1 px kenarlığı için iki yandan 1'er piksel pay
    width: surfaceItem.width + 2
    height: titleBarHeight + surfaceItem.height + 2

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
}
