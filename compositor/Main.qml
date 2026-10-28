// ankaros compositor'ı: Wayland sunucusu, tam ekran masaüstü (Ankaros.Shell) ve pencere yönetimi.
// --login ile açılırsa ekran giriş ekranıdır (docs/oturum.md).
pragma ComponentBehavior: Bound

import QtQuick
import QtWayland.Compositor
import QtWayland.Compositor.XdgShell

import Ankaros.Shell
import Ankaros.System
import Ankaros.UI

WaylandCompositor {
    id: compositor

    // Açık pencereler (WaylandWindow), açılış sırasıyla
    property var windows: []

    readonly property var runningApps: windows.map(w => w.appName)
    readonly property string activeApp: windows.find(w => w.active)?.appName ?? ""

    // Ayarlar uygulamasının bir sayfası istendiğinde: penceresi açıksa öne getir, değilse o sayfayla başlat
    function openSettings(page) {
        const own = windows.filter(w => w.appName === "Ayarlar");
        if (own.length > 0) {
            raise(own[own.length - 1]);
            own[own.length - 1].takeFocus();
        } else {
            AppLauncher.launch(AppCatalog.command("Ayarlar") + " --page " + page, socketName);
        }
    }

    function raise(window) {
        window.minimized = false;
        // Kabuğun Ayarlar penceresiyle aynı sıralamayı paylaşmak için z değeri Desktop'tan alınır.
        window.z = desktop.nextZ();
    }

    // Alt çubuktan ya da başlat panelinden bir uygulamaya tıklandığında: penceresi yoksa başlat,
    // odaktaysa küçült, değilse öne getir.
    function activate(app) {
        const own = windows.filter(w => w.appName === app);
        if (own.length === 0) {
            const command = AppCatalog.command(app);
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
        // Retina ekranlarda 2 (oturum yöneticisi QT_SCALE_FACTOR=2 verir). Uygulamalar bu ölçeği
        // Wayland çıktısından öğrenir ve pencerelerini 2 kat çözünürlükle çizer.
        scaleFactor: Math.max(1, Math.round(screen.pixelRatio))

        window: Window {
            id: screen

            // Açılış süresi ölçümü: ilk kare çizilince init'e bildirilir (ankarosctl boot, docs/acilis-suresi.md)
            property bool firstFrameReported: false
            readonly property real pixelRatio: Screen.devicePixelRatio

            visible: true
            visibility: Window.FullScreen
            color: "black"
            onFrameSwapped: {
                if (!firstFrameReported) {
                    firstFrameReported = true;
                    Session.markBoot(Session.greeter ? "giriş ekranı" : "masaüstü");
                }
            }

            Desktop {
                id: desktop

                anchors.fill: parent
                // Uygulama listesi /etc/ankaros/apps.json'dan (AppCatalog)
                apps: AppCatalog.names
                taskBarApps: {
                    // Sabitlenmiş uygulamalar ve sırayla, sabitlenmemiş ama çalışan uygulamalar
                    // Kullanıcının seçimi (Ayarlar › Alt çubuk), seçim yoksa apps.json'daki varsayılan
                    const pinned = Appearance.customPinnedApps ? Appearance.pinnedApps : AppCatalog.pinned;
                    return pinned.concat(compositor.runningApps.filter(
                        (app, i, all) => !pinned.includes(app) && all.indexOf(app) === i));
                }
                running: compositor.runningApps
                activeApp: compositor.activeApp
                userName: Session.userName
                session: Session
                network: Network
                visible: !Session.greeter
                onAppLaunched: app => compositor.activate(app)
                onSettingsRequested: page => compositor.openSettings(page)
            }

            // Giriş ekranı (ankaros-compositor --login). Şifre doğruysa oturum yöneticisi bu süreci
            // kapatır ve kullanıcının masaüstünü açar.
            Loader {
                anchors.fill: parent
                active: Session.greeter

                sourceComponent: LoginScreen {
                    id: login

                    userName: Session.userName
                    onSubmitted: password => Session.login(Session.userName, password)
                    onRebootRequested: Session.reboot()
                    onPoweroffRequested: Session.poweroff()

                    Connections {
                        target: Session

                        function onLoginFailed(message: string) {
                            login.fail(message);
                        }
                    }
                }
            }

            Shortcut {
                sequence: "Meta+L"
                enabled: !Session.greeter
                onActivated: desktop.lock()
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
            window.appName = Qt.binding(() => AppCatalog.nameForAppId(window.appId));
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
