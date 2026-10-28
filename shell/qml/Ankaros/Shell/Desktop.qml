// Masaüstünün kökü: duvar kağıdı, alt çubuk ve başlat paneli.
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
    // Başlat panelindeki öneriler: [{ app, title, subtitle }, ...]
    property var recommended: []
    property string userName
    // Oturum işlemleri: kilit açma, oturumu kapatma, kapatma, otomatik giriş, şifre (docs/oturum.md).
    // Compositor'da C++ Session nesnesi, Mac önizlemesinde sahte bir oturum verilir. Beklenen üyeler:
    // userName, autologin, unlock(), logout(), reboot(), poweroff(), setAutologin(), changePassword()
    // ve unlocked, unlockFailed, passwordUpdated, passwordUpdateFailed sinyalleri.
    property var session: null
    // Kablolu ağın durumu ve ayarı (docs/ag.md). Compositor'da C++ Network nesnesi, önizlemede sahtesi.
    // Beklenen üyeler: available, interfaceName, carrier, connected, mode, address, gateway, dns, mac,
    // configAddress, configGateway, configDns, apply() ve applied, applyFailed sinyalleri.
    property var network: null
    // Ekran kilitli mi. lock() kilitler, kilidi doğru şifre açar.
    property bool locked: false

    // Başlat panelinin açık olup olmadığı. Compositor'da Super tuşu bunu değiştirecek.
    property alias startMenuOpen: startMenu.open
    // Uygulama pencerelerinin yerleştiği katman: duvar kağıdının üstünde, alt çubuğun altında.
    // Compositor pencere çerçevelerini bu öğenin çocuğu olarak oluşturur.
    readonly property alias windowLayer: windowLayer
    // Pencerelerin kullanabileceği alan (alt çubuk hariç)
    readonly property rect workArea: Qt.rect(0, 0, width, taskBar.y - Theme.size.taskBarMargin)

    // Pencerelerin üst üste sırası. Compositor pencereleri öne getirirken bunu kullanır.
    property int topZ: 0

    // Kullanıcı bir uygulamayı başlattığında. Compositor bunu dinleyip uygulamayı çalıştırır ya da
    // penceresi açıksa öne getirir.
    signal appLaunched(string app)
    // Ayarlar uygulamasının belirli bir sayfası istendiğinde (ör. alt çubuktaki ağ simgesi: "network")
    signal settingsRequested(string page)

    function nextZ() {
        return ++topZ;
    }

    function lock() {
        startMenu.open = false;
        locked = true;
    }

    function launch(app) {
        appLaunched(app);
    }

    // Duvar kağıdı ve pencereler. Alt çubuk ve başlat paneli buzlu cam efekti için bu öğenin
    // görüntüsünü bulanıklaştırır, böylece arkalarındaki pencereler de bulanık görünür.
    Item {
        id: scene

        anchors.fill: parent

        Wallpaper {
            anchors.fill: parent
        }

        Item {
            id: windowLayer

            anchors.fill: parent
        }
    }

    // Başlat paneli açıkken dışarı tıklamak paneli kapatır.
    MouseArea {
        anchors.fill: parent
        enabled: startMenu.open
        onClicked: startMenu.open = false
    }

    StartMenu {
        id: startMenu

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: taskBar.top
        anchors.bottomMargin: Theme.size.taskBarMargin
        backdrop: scene
        apps: root.apps
        recommended: root.recommended
        userName: root.userName
        onAppLaunched: app => {
            open = false;
            root.launch(app);
        }
        onLockRequested: root.lock()
        onLogoutRequested: root.session?.logout()
        onRebootRequested: root.session?.reboot()
        onPoweroffRequested: root.session?.poweroff()
    }

    TaskBar {
        id: taskBar

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Theme.size.taskBarMargin
        backdrop: scene
        apps: root.taskBarApps
        running: root.running
        activeApp: root.activeApp
        startMenuOpen: startMenu.open
        onStartClicked: startMenu.open = !startMenu.open
        onSearchClicked: {
            startMenu.open = true;
            startMenu.focusSearch();
        }
        network: root.network
        onAppClicked: app => root.launch(app)
        onNetworkClicked: root.settingsRequested("network")
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
