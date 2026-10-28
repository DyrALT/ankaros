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

    // Kabuğun kendi Ayarlar penceresi açık mı, odakta mı. Bu pencere bir Wayland uygulaması değil,
    // kabuğun parçası. Bu yüzden çalışma ve odak durumu burada tutulur.
    property bool settingsOpen: false
    property bool settingsFocused: false
    // Ayarlar penceresinde açık sayfa ("appearance", "user")
    property alias settingsPanelPage: settingsPanel.page

    // Pencerelerin üst üste sırası. Compositor da kendi pencerelerini öne getirirken bunu kullanır,
    // böylece Ayarlar penceresiyle uygulama pencereleri aynı sıralamayı paylaşır.
    property int topZ: 0

    // Kullanıcı bir uygulamayı başlattığında. Compositor bunu dinleyip uygulamayı çalıştıracak.
    // "Ayarlar" bu sinyale düşmez, kabuk onu kendisi açar.
    signal appLaunched(string app)

    function nextZ() {
        return ++topZ;
    }

    function lock() {
        startMenu.open = false;
        locked = true;
    }

    function launch(app) {
        if (app !== "Ayarlar") {
            appLaunched(app);
            return;
        }
        if (settingsOpen && settingsFocused) {
            settingsOpen = false;
            return;
        }
        settingsOpen = true;
        settingsFocused = true;
        settingsWindow.z = nextZ();  // zaten açıksa da öne getir
    }

    // Başka bir uygulama penceresi odak alınca Ayarlar penceresi odaktan çıkar.
    onActiveAppChanged: settingsFocused = false
    // Ayarlar penceresi nasıl açılırsa açılsın en öne gelsin.
    onSettingsOpenChanged: {
        if (settingsOpen)
            settingsWindow.z = nextZ();
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

            WindowChrome {
                id: settingsWindow

                visible: root.settingsOpen
                width: Math.min(820, root.width - 80)
                height: Math.min(600, root.workArea.height - 40)
                x: (root.width - width) / 2
                y: Math.max(20, (root.workArea.height - height) / 2)
                app: "Ayarlar"
                title: qsTr("Ayarlar")
                active: root.settingsFocused
                onPressed: {
                    z = root.nextZ();
                    root.settingsFocused = true;
                }
                onCloseClicked: root.settingsOpen = false
                onMinimizeClicked: root.settingsOpen = false

                SettingsPanel {
                    id: settingsPanel

                    anchors.fill: parent
                    session: root.session
                    network: root.network
                }
            }
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
        apps: root.settingsOpen && !root.taskBarApps.includes("Ayarlar")
              ? root.taskBarApps.concat(["Ayarlar"]) : root.taskBarApps
        running: root.settingsOpen ? root.running.concat(["Ayarlar"]) : root.running
        activeApp: root.settingsFocused ? "Ayarlar" : root.activeApp
        startMenuOpen: startMenu.open
        onStartClicked: startMenu.open = !startMenu.open
        onSearchClicked: {
            startMenu.open = true;
            startMenu.focusSearch();
        }
        network: root.network
        onAppClicked: app => root.launch(app)
        onNetworkClicked: {
            root.settingsPanelPage = "network";
            if (!root.settingsOpen || !root.settingsFocused)
                root.launch("Ayarlar");
        }
    }

    // Kilit ekranı her şeyin üstünde. Arkasındaki sahne bulanık görünür.
    LockScreen {
        id: lockScreen

        anchors.fill: parent
        opacity: root.locked ? 1 : 0
        backdrop: scene
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
