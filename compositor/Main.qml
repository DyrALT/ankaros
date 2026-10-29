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
    // Pencereler son odaklanandan başlayarak (Alt+Tab bu sırayla gezer)
    property var focusHistory: []
    // Alt+Tab sürerken gösterilen pencereler ve seçili olanın sırası (-1: kapalı)
    property var switcherList: []
    property int switcherIndex: -1
    // Süper+D'nin küçülttüğü pencereler, ikinci basışta geri gelir
    property var hiddenByShowDesktop: []

    readonly property var runningApps: windows.map(w => w.appName)
    readonly property string activeApp: windows.find(w => w.active)?.appName ?? ""

    // Ayarlar uygulamasının bir sayfası istendiğinde: penceresi açıksa öne getir, değilse o sayfayla başlat
    function openSettings(page) {
        const own = windows.filter(w => w.appName === "Ayarlar");
        Appearance.noteAppUsed("Ayarlar");
        if (own.length > 0) {
            // Açık Ayarlar penceresi shell.conf'u izliyor, isteği görünce sayfaya geçer
            Appearance.requestSettingsPage(page);
            raise(own[own.length - 1]);
            own[own.length - 1].takeFocus();
        } else {
            AppLauncher.launch(AppCatalog.command("Ayarlar") + " --page " + page, socketName);
        }
    }

    // Bir noktada en üstte görünen pencere. margin: çerçevenin dışındaki boyutlandırma tutamakları da
    // sayılsın diye pencerenin çevresinde ne kadar pay bırakılacağı
    function windowAt(position, margin = 0) {
        let top = null;
        for (const w of windows) {
            const m = w.resizable ? margin : 0;
            if (w.visible && !w.closing && position.x >= w.x - m && position.x < w.x + w.width + m
                    && position.y >= w.y - m && position.y < w.y + w.height + m && (!top || w.z > top.z))
                top = w;
        }
        return top;
    }

    function activeWindow() {
        return windows.find(w => w.active && !w.minimized) ?? null;
    }

    // Küçültülen pencerenin odağını en son kullanılan görünür pencereye ver
    function focusNext(previous) {
        const next = focusHistory.find(w => w !== previous && !w.minimized);
        if (next)
            next.takeFocus();
    }

    // Süper+D: bütün pencereleri küçült, ikinci basışta geri getir
    function showDesktop() {
        const shown = windows.filter(w => !w.minimized);
        if (shown.length > 0) {
            hiddenByShowDesktop = shown;
            shown.forEach(w => w.minimized = true);
            return;
        }
        const hidden = hiddenByShowDesktop.filter(w => windows.includes(w)).sort((a, b) => a.z - b.z);
        hidden.forEach(w => raise(w));
        if (hidden.length > 0)
            hidden[hidden.length - 1].takeFocus();
        hiddenByShowDesktop = [];
    }

    function switchWindow(backward) {
        if (switcherIndex < 0) {
            const list = focusHistory.concat(windows.filter(w => !focusHistory.includes(w)));
            if (list.length === 0)
                return;
            switcherList = list;
            // İlk basışta bir önceki pencere seçilir, Alt hemen bırakılırsa ikisi yer değiştirir.
            switcherIndex = list.length === 1 ? 0 : backward ? list.length - 1 : 1;
        } else {
            const count = switcherList.length;
            switcherIndex = (switcherIndex + (backward ? -1 : 1) + count) % count;
        }
    }

    function finishSwitch(commit) {
        const window = switcherList[switcherIndex];
        switcherIndex = -1;
        switcherList = [];
        if (commit && window && windows.includes(window)) {
            raise(window);
            window.takeFocus();
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
        Appearance.noteAppUsed(app);
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
                // Uygulama listesi /etc/ankaros/apps.json'dan (AppCatalog). Komutu olmayan (henüz
                // yazılmamış) uygulamalar gösterilmez.
                apps: AppCatalog.launchable
                taskBarApps: {
                    // Sabitlenmiş uygulamalar ve sırayla, sabitlenmemiş ama çalışan uygulamalar
                    // Kullanıcının seçimi (Ayarlar › Alt çubuk), seçim yoksa apps.json'daki varsayılan
                    const pinned = (Appearance.customPinnedApps ? Appearance.pinnedApps : AppCatalog.pinned)
                        .filter(app => AppCatalog.launchable.includes(app));
                    return pinned.concat(compositor.runningApps.filter(
                        (app, i, all) => !pinned.includes(app) && all.indexOf(app) === i));
                }
                recentApps: Appearance.recentApps.filter(recent => AppCatalog.launchable.includes(recent.app))
                running: compositor.runningApps
                activeApp: compositor.activeApp
                userName: Session.userName
                session: Session
                network: Network
                battery: Battery
                visible: !Session.greeter
                snapPreview: {
                    const dragged = compositor.windows.find(w => w.snapZone !== "");
                    return dragged ? dragged.snapRect : Qt.rect(0, 0, 0, 0);
                }
                switcherWindows: compositor.switcherList.map(w => ({
                    app: w.appName,
                    title: w.title,
                    item: w,
                    minimized: w.minimized
                }))
                switcherIndex: compositor.switcherIndex
                onAppLaunched: app => compositor.activate(app)
                onSettingsRequested: page => compositor.openSettings(page)
                onSwitcherActivated: index => {
                    compositor.switcherIndex = index;
                    compositor.finishSwitch(true);
                }
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

            // Kısayollar (docs/faz-3-plani.md › Pencere yönetimi). Süper tek başına ve Alt+Tab
            // GlobalKeys'te, çünkü Shortcut tuşun bırakılmasını göremiyor.
            Shortcut {
                sequence: "Meta+L"
                enabled: !Session.greeter
                onActivated: desktop.lock()
            }

            Shortcut {
                sequence: "Meta+D"
                enabled: GlobalKeys.enabled
                onActivated: compositor.showDesktop()
            }

            Shortcut {
                sequence: "Meta+Left"
                enabled: GlobalKeys.enabled
                onActivated: compositor.activeWindow()?.tile("left")
            }

            Shortcut {
                sequence: "Meta+Right"
                enabled: GlobalKeys.enabled
                onActivated: compositor.activeWindow()?.tile("right")
            }

            Shortcut {
                sequence: "Meta+Up"
                enabled: GlobalKeys.enabled
                onActivated: {
                    const window = compositor.activeWindow();
                    if (window && !window.maximized)
                        window.maximize();
                }
            }

            Shortcut {
                sequence: "Meta+Down"
                enabled: GlobalKeys.enabled
                onActivated: {
                    const window = compositor.activeWindow();
                    if (!window)
                        return;
                    if (window.maximized || window.tiled !== "")
                        window.restore();
                    else
                        window.minimized = true;
                }
            }

            Shortcut {
                sequence: "Alt+F4"
                enabled: GlobalKeys.enabled
                onActivated: compositor.activeWindow()?.toplevel.sendClose()
            }

            Connections {
                target: GlobalKeys

                function onSuperTapped() {
                    desktop.toggleStartMenu();
                }

                function onSwitchRequested(backward: bool) {
                    compositor.switchWindow(backward);
                }

                function onSwitchCommitted() {
                    compositor.finishSwitch(true);
                }

                function onSwitchCancelled() {
                    compositor.finishSwitch(false);
                }

                // Bir pencerenin herhangi bir yerine (uygulamanın içi dahil) basılınca öne gelsin ve
                // klavye odağını alsın. Alt çubuğa ya da açık bir panele basıldıysa dokunma.
                function onPointerPressed(position: point) {
                    if (Session.greeter || desktop.locked || desktop.shellAt(position))
                        return;
                    const window = compositor.windowAt(position, 6);
                    if (!window)
                        return;
                    if (window.z !== desktop.topZ)
                        compositor.raise(window);
                    window.takeFocus();
                }
            }

            // Klavye düzeni (Ayarlar › Klavye ve dil): uygulamalara Wayland klavye haritasıyla,
            // compositor'ın kendi arayüzüne KeyboardLayout ile anında uygulanır.
            Binding {
                target: KeyboardLayout
                property: "layout"
                value: Appearance.keyboardLayout
            }

            Binding {
                target: compositor.defaultSeat ? compositor.defaultSeat.keymap : null
                property: "layout"
                value: KeyboardLayout.xkbLayout
            }

            Binding {
                target: compositor.defaultSeat ? compositor.defaultSeat.keymap : null
                property: "variant"
                value: KeyboardLayout.xkbVariant
            }

            Binding {
                target: GlobalKeys
                property: "enabled"
                value: !Session.greeter && !desktop.locked
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
            window.minimizeTarget = () => desktop.taskIconCenter(window.appName);
            window.isTopAt = position => compositor.windowAt(position) === window;
            window.raiseRequested.connect(() => compositor.raise(window));
            window.activeChanged.connect(() => {
                if (window.active)
                    compositor.focusHistory = [window].concat(compositor.focusHistory.filter(w => w !== window));
            });
            window.minimizedChanged.connect(() => {
                if (window.minimized)
                    compositor.focusNext(window);
            });
            window.gone.connect(() => {
                compositor.windows = compositor.windows.filter(w => w !== window);
                compositor.focusHistory = compositor.focusHistory.filter(w => w !== window);
                if (compositor.switcherList.includes(window))
                    compositor.finishSwitch(false);
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
