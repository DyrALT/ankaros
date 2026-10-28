// ankaros compositor'ı: Wayland sunucusu, tam ekran masaüstü (Ankaros.Shell) ve pencere yönetimi.
pragma ComponentBehavior: Bound

import QtQuick
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell

import Ankaros.Shell

WaylandCompositor {
    id: compositor

    // Uygulama adı -> çalıştırılacak komut ve Wayland app_id -> uygulama adı.
    // Faz 1 C'de /etc/ankaros/apps.json'dan okunacak.
    readonly property var commands: ({
            "Terminal": "foot"
        })
    readonly property var appNames: ({
            "foot": "Terminal"
        })

    // Açık pencereler (WaylandWindow), açılış sırasıyla
    property var windows: []
    property int topZ: 0

    readonly property var runningApps: windows.map(w => w.appName)
    readonly property string activeApp: windows.find(w => w.active)?.appName ?? ""

    function appNameFor(appId) {
        return appNames[appId] ?? appId;
    }

    function raise(window) {
        window.minimized = false;
        window.z = ++topZ;
    }

    // Alt çubuktan ya da başlat panelinden bir uygulamaya tıklandığında: penceresi yoksa başlat,
    // odaktaysa küçült, değilse öne getir.
    function activate(app) {
        const own = windows.filter(w => w.appName === app);
        if (own.length === 0) {
            const command = commands[app];
            if (command)
                AppLauncher.launch(command, socketName);
            else
                console.warn("Bu uygulama için komut tanımlı değil:", app);
            return;
        }
        const window = own[own.length - 1];
        if (window.active && !window.minimized) {
            window.minimized = true;
        } else {
            raise(window);
            window.takeFocus();
        }
    }

    socketName: "wayland-0"

    WaylandOutput {
        sizeFollowsWindow: true

        window: Window {
            visible: true
            visibility: Window.FullScreen
            color: "black"

            Desktop {
                id: desktop

                anchors.fill: parent
                apps: ["Terminal", "Dosyalar", "Tarayıcı", "Ayarlar", "Metin Düzenleyici", "Müzik",
                    "Fotoğraflar", "Takvim", "Hesap Makinesi", "Sistem İzleyici", "Notlar", "Paket Merkezi"]
                taskBarApps: {
                    // Sabitlenmiş uygulamalar ve sırayla, sabitlenmemiş ama çalışan uygulamalar
                    const pinned = ["Terminal", "Dosyalar", "Tarayıcı", "Metin Düzenleyici"];
                    return pinned.concat(compositor.runningApps.filter(
                        (app, i, all) => !pinned.includes(app) && all.indexOf(app) === i));
                }
                running: compositor.runningApps
                activeApp: compositor.activeApp
                userName: "root"
                onAppLaunched: app => compositor.activate(app)
            }
        }
    }

    Component {
        id: windowComponent

        WaylandWindow {}
    }

    XdgShell {
        onToplevelCreated: (toplevel, xdgSurface) => {
            // Yeni pencereler hafifçe kaydırılarak üst üste açılır.
            const offset = 32 * (compositor.windows.length % 8);
            const window = windowComponent.createObject(desktop.windowLayer, {
                xdgSurface: xdgSurface,
                x: 160 + offset,
                y: 60 + offset
            });
            window.workArea = Qt.binding(() => desktop.workArea);
            window.appName = Qt.binding(() => compositor.appNameFor(window.appId));
            window.raiseRequested.connect(() => compositor.raise(window));
            window.gone.connect(() => {
                compositor.windows = compositor.windows.filter(w => w !== window);
            });
            compositor.windows = compositor.windows.concat([window]);
            compositor.raise(window);
            window.takeFocus();
        }
    }

    // Uygulamalardan kendi başlık çubuklarını çizmemelerini iste, çerçeveyi biz çiziyoruz (karar 0007).
    XdgDecorationManagerV1 {
        preferredMode: XdgToplevel.ServerSideDecoration
    }
}
