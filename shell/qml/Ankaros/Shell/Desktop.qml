// Masaüstünün kökü: duvar kağıdı, alt çubuk, başlat paneli, takvim ve hızlı ayarlar.
// Mac önizlemesi bunu bir pencereye, compositor (Faz 1) ise ekranın kendisine yerleştirir.
// Wayland'e özgü hiçbir tür import edilmez. Dış dünyadan gelen veriler property olarak verilir.
import QtQuick

import Ankaros.UI

Item {
    id: root

    // Sabitlenmiş uygulamalar, başlat panelinde gösterilir: ["Terminal", ...]
    property var apps: []
    // Alt çubukta gösterilecek uygulamalar (sabitlenmiş + çalışanlar)
    property var taskBarApps: []
    // Penceresi açık uygulamalar ve odaktaki uygulama
    property var running: []
    property string activeApp
    // Son kullanılan uygulamalar, en yeniden eskiye: [{ app, time }, ...] (Appearance.recentApps)
    property var recentApps: []
    property string userName
    // Oturum işlemleri: kilit açma, oturumu kapatma, kapatma, otomatik giriş, şifre (docs/oturum.md).
    // Compositor'da C++ Session nesnesi, Mac önizlemesinde sahte bir oturum verilir. Beklenen üyeler:
    // userName, autologin, lock(), unlock(), logout(), reboot(), poweroff(), setAutologin(), changePassword()
    // ve unlocked, unlockFailed, passwordUpdated, passwordUpdateFailed sinyalleri.
    property var session: null
    // Kablolu ağın durumu ve ayarı (docs/ag.md). Compositor'da C++ Network nesnesi, önizlemede sahtesi.
    // Beklenen üyeler: available, interfaceName, carrier, connected, mode, address, gateway, dns, mac,
    // configAddress, configGateway, configDns, apply() ve applied, applyFailed sinyalleri.
    property var network: null
    // Pil (Ankaros.System Battery): present, level, charging. Pil yoksa alt çubukta simgesi görünmez.
    property var battery: null
    // Ses (Ankaros.System Audio): available, volume (0-100), muted. Ses aygıtı yoksa simgesi görünmez.
    property var audio: null
    // Ekran kilitli mi. lock() kilitler, kilidi doğru şifre açar.
    property bool locked: false

    // Alt çubuğun açılır panelleri. Biri açılınca diğerleri kapanır.
    property alias startMenuOpen: startMenu.open
    property alias calendarOpen: calendar.open
    property alias quickSettingsOpen: quickSettings.open
    // Masaüstüne sağ tıklayınca açılan menü
    property alias contextMenuOpen: desktopMenu.open
    // Uygulama pencerelerinin yerleştiği katman: duvar kağıdının üstünde, alt çubuğun altında.
    // Compositor pencere çerçevelerini bu öğenin çocuğu olarak oluşturur.
    readonly property alias windowLayer: windowLayer
    // Pencerelerin kullanabileceği alan (alt çubuk hariç). Alt çubuk gizlenince değişmez: gizleme
    // yalnızca kaydırmadır (transform), taskBar.y aynı kalır.
    readonly property rect workArea: Qt.rect(0, 0, width, taskBar.y - Theme.size.taskBarMargin)
    // Öndeki pencere tam ekran mı (compositor verir). Öyleyse alt çubuk aşağı kayıp gizlenir, fare
    // ekranın en altına inince geri çıkar.
    property bool fullscreen: false
    readonly property bool taskBarHidden: fullscreen && !taskBarPeeking
    // Tam ekranda alt çubuk şu an gösteriliyor mu (fare altta, alt çubuğun üstünde ya da bir paneli açık)
    property bool taskBarPeeking: false
    readonly property bool taskBarWanted: bottomEdgeHover.hovered || taskBarHover.hovered || startMenu.open
                                          || calendar.open || quickSettings.open

    // Pencerelerin üst üste sırası. Compositor pencereleri öne getirirken bunu kullanır.
    property int topZ: 0
    // Sürüklenen pencere bırakılınca kaplayacağı yer (yaslama önizlemesi). Boşsa gizli.
    property rect snapPreview
    // Alt+Tab: son kullanılandan başlayarak pencereler ({ app, title, item, minimized }) ve seçili
    // olanın sırası (-1: kapalı)
    property var switcherWindows: []
    property int switcherIndex: -1

    // Kullanıcı bir uygulamayı başlattığında. Compositor bunu dinleyip uygulamayı çalıştırır ya da
    // penceresi açıksa öne getirir.
    signal appLaunched(string app)
    // Ayarlar uygulamasının belirli bir sayfası istendiğinde (ör. alt çubuktaki ağ simgesi: "network")
    signal settingsRequested(string page)
    // Alt+Tab geçişinde bir pencereye tıklandı
    signal switcherActivated(int index)

    function nextZ() {
        return ++topZ;
    }

    function closePanels() {
        startMenu.open = false;
        calendar.open = false;
        quickSettings.open = false;
        desktopMenu.open = false;
    }

    // Masaüstü menüsünü bir noktada aç (sahne koordinatları). Başka paneller kapanır, menü ekranın
    // ve alt çubuğun dışına taşmaz.
    function showContextMenu(position) {
        if (locked || switcher.visible)
            return;
        closePanels();
        desktopMenu.x = Math.max(Theme.spacing.sm, Math.min(position.x, width - desktopMenu.width - Theme.spacing.sm));
        desktopMenu.y = Math.max(Theme.spacing.sm, Math.min(position.y, workArea.height - desktopMenu.height));
        desktopMenu.open = true;
    }

    // Panellerden birini aç ya da açıksa kapat, diğerlerini kapat
    function togglePanel(panel) {
        const wasOpen = panel.open;
        closePanels();
        panel.open = !wasOpen;
    }

    function toggleStartMenu() {
        togglePanel(startMenu);
    }

    // Başlat panelini açıp bir görünümü göster: "all" (bütün uygulamalar) ya da arama metni
    function showStartMenu(view, query) {
        closePanels();
        startMenu.open = true;
        startMenu.view = view;
        startMenu.setQuery(query);
    }

    function lock() {
        closePanels();
        locked = true;
        if (session)
            session.lock();
    }

    function launch(app) {
        appLaunched(app);
    }

    // Sahnedeki bir nokta masaüstünün kendi arayüzünde mi (alt çubuk, açık paneller, pencere geçişi).
    // Öyleyse o noktaya tıklamak arkadaki pencereyi öne getirmez.
    function shellAt(position) {
        if (switcher.visible || root.locked)
            return true;
        for (const item of [taskBar, startMenu, calendar, quickSettings, desktopMenu]) {
            if (item.visible && item.contains(item.mapFromItem(null, position)))
                return true;
        }
        // Açık bir panelin ya da menünün dışına tıklamak onu kapatır, pencereye ulaşmaz
        return startMenu.open || calendar.open || quickSettings.open || desktopMenu.open;
    }

    // Alt çubukta uygulamanın simgesinin ortası (küçültme animasyonu buraya uçar)
    function taskIconCenter(app) {
        return taskBar.iconCenter(app);
    }

    // Duvar kağıdı ve pencereler. Alt çubuk ve başlat paneli buzlu cam efekti için bu öğenin
    // görüntüsünü bulanıklaştırır, böylece arkalarındaki pencereler de bulanık görünür.
    Item {
        id: scene

        anchors.fill: parent

        Wallpaper {
            anchors.fill: parent
        }

        // Boş masaüstüne sağ tık. Pencereler bunun üstünde olduğu için pencerenin içindeki sağ tık
        // uygulamaya gider. Basınca açılır: kısa tıklamalar yavaş çizimde TapHandler'da kaybolabiliyor.
        MouseArea {
            anchors.fill: parent
            acceptedButtons: Qt.RightButton
            onPressed: mouse => root.showContextMenu(Qt.point(mouse.x, mouse.y))
        }

        Item {
            id: windowLayer

            anchors.fill: parent

            // Yaslama önizlemesi: sürüklenen pencerenin hemen altında
            Rectangle {
                id: snapIndicator

                readonly property bool shown: root.snapPreview.width > 0

                x: shown ? root.snapPreview.x + Theme.spacing.sm : x
                y: shown ? root.snapPreview.y + Theme.spacing.sm : y
                width: shown ? root.snapPreview.width - 2 * Theme.spacing.sm : width
                height: shown ? root.snapPreview.height - 2 * Theme.spacing.sm : height
                z: root.topZ - 0.5
                radius: Theme.radius.lg
                color: Qt.alpha(Theme.color.accent, 0.18)
                border.width: 2
                border.color: Qt.alpha(Theme.color.accent, 0.7)
                opacity: shown ? 1 : 0
                visible: opacity > 0

                Behavior on opacity {
                    NumberAnimation { duration: Theme.motion.fast }
                }

                Behavior on width {
                    enabled: snapIndicator.visible
                    NumberAnimation { duration: Theme.motion.fast; easing.type: Theme.motion.easing }
                }

                Behavior on x {
                    enabled: snapIndicator.visible
                    NumberAnimation { duration: Theme.motion.fast; easing.type: Theme.motion.easing }
                }
            }
        }
    }

    // Açılır paneller açıkken dışarı tıklamak onları kapatır. Her MouseArea'nın bir imleci vardır:
    // görünür kalırsa pencerelerin üstünde durduğu için tutamakların imlecini örter.
    // Sağ tık da kapatır (menü açıkken başka yere sağ tıklamak onu kapatır, yenisi açılmaz).
    MouseArea {
        anchors.fill: parent
        visible: startMenu.open || calendar.open || quickSettings.open || desktopMenu.open
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        onPressed: root.closePanels()
    }

    Shortcut {
        sequence: "Escape"
        enabled: desktopMenu.open
        onActivated: desktopMenu.open = false
    }

    DesktopMenu {
        id: desktopMenu

        backdrop: scene
        onWallpaperRequested: {
            open = false;
            root.settingsRequested("wallpaper");
        }
        onAppearanceRequested: {
            open = false;
            root.settingsRequested("appearance");
        }
        onThemeToggleRequested: {
            open = false;
            Appearance.darkMode = !Appearance.darkMode;
        }
        onTerminalRequested: {
            open = false;
            root.launch("Terminal");
        }
        onLockRequested: {
            open = false;
            root.lock();
        }
    }

    StartMenu {
        id: startMenu

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: taskBar.top
        anchors.bottomMargin: Theme.size.taskBarMargin
        backdrop: scene
        apps: root.apps
        recentApps: root.recentApps
        userName: root.userName
        onAppLaunched: app => {
            open = false;
            root.launch(app);
        }
        onSettingsRequested: page => {
            open = false;
            root.settingsRequested(page);
        }
        onLockRequested: root.lock()
        onLogoutRequested: root.session?.logout()
        onRebootRequested: root.session?.reboot()
        onPoweroffRequested: root.session?.poweroff()
    }

    // Tam ekranda alt çubuğu geri getiren ince şerit: ekranın en altı. Fare olaylarını tüketmez
    // (HoverHandler), alttaki pencere tıklamaları almaya devam eder.
    Item {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 2
        visible: root.fullscreen

        HoverHandler {
            id: bottomEdgeHover
        }
    }

    onTaskBarWantedChanged: {
        if (taskBarWanted) {
            taskBarHideDelay.stop();
            taskBarPeeking = true;
        } else {
            taskBarHideDelay.restart();
        }
    }

    // Fare alt çubuktan ayrılınca hemen kaçmasın, kısa bir süre sonra insin
    Timer {
        id: taskBarHideDelay

        interval: 500
        onTriggered: root.taskBarPeeking = false
    }

    TaskBar {
        id: taskBar

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Theme.size.taskBarMargin
        // Gizlenince ekranın altına kayar. Yer değiştirmez (y aynı), workArea etkilenmez.
        transform: Translate {
            y: root.taskBarHidden ? taskBar.height + 2 * Theme.size.taskBarMargin : 0

            Behavior on y {
                NumberAnimation { duration: Theme.motion.normal; easing.type: Theme.motion.easing }
            }
        }
        backdrop: scene
        apps: root.taskBarApps
        running: root.running
        activeApp: root.activeApp
        startMenuOpen: startMenu.open
        calendarOpen: calendar.open
        quickSettingsOpen: quickSettings.open
        onStartClicked: root.togglePanel(startMenu)
        onSearchClicked: {
            root.closePanels();
            startMenu.open = true;
            startMenu.focusSearch();
        }
        network: root.network
        battery: root.battery
        audio: root.audio
        onAppClicked: app => {
            root.closePanels();
            root.launch(app);
        }
        onStatusClicked: root.togglePanel(quickSettings)
        onClockClicked: root.togglePanel(calendar)

        // Tam ekranda fare alt çubuğun üstündeyken gizlenmesin
        HoverHandler {
            id: taskBarHover
        }
    }

    CalendarPopup {
        id: calendar

        anchors.right: parent.right
        anchors.bottom: taskBar.top
        anchors.margins: Theme.size.taskBarMargin
        backdrop: scene
    }

    QuickSettings {
        id: quickSettings

        anchors.right: parent.right
        anchors.bottom: taskBar.top
        anchors.margins: Theme.size.taskBarMargin
        backdrop: scene
        userName: root.userName
        network: root.network
        audio: root.audio
        onNetworkClicked: {
            open = false;
            root.settingsRequested("network");
        }
        onSettingsClicked: {
            open = false;
            root.launch("Ayarlar");
        }
        onLockRequested: root.lock()
        onLogoutRequested: root.session?.logout()
        onRebootRequested: root.session?.reboot()
        onPoweroffRequested: root.session?.poweroff()
    }

    WindowSwitcher {
        id: switcher

        anchors.fill: parent
        backdrop: scene
        windows: root.switcherWindows
        currentIndex: root.switcherIndex
        onActivated: index => root.switcherActivated(index)
    }

    // Kilit ekranı her şeyin üstünde. Arkasındaki sahne bulanık görünür.
    // Kilit ekranının arka planı, kullanıcı duvar kağıdını kilitte istemiyorsa (Ayarlar › Duvar kağıdı)
    Wallpaper {
        id: defaultWallpaper

        anchors.fill: parent
        visible: false
        name: "ankara-tasi"
    }

    LockScreen {
        id: lockScreen

        anchors.fill: parent
        opacity: root.locked ? 1 : 0
        backdrop: Appearance.wallpaperOnLock ? scene : defaultWallpaper
        userName: root.userName
        onSubmitted: password => {
            if (root.session)
                root.session.unlock(password);
            else
                root.locked = false;
        }
    }

    Connections {
        target: root.session
        ignoreUnknownSignals: true

        function onUnlocked() {
            root.locked = false;
        }

        function onUnlockFailed(message: string) {
            lockScreen.fail(message);
        }
    }
}
