// Önizleme penceresi. Açılış boyutu Figma'daki masaüstü çerçeveleriyle aynıdır (1440x900),
// pencere yeniden boyutlandırılabilir ve tam ekran yapılabilir. Kabuk her boyuta uyum sağlar.
//
// Kısayollar (macOS'ta Ctrl yerine Cmd):
//   Ctrl+R  yeniden derleyip yeniden aç: uygulama 42 koduyla çıkar, scripts/shell-preview.sh bunu
//           görünce derleyip uygulamayı yeniden başlatır (tema ayarı kalıcı olduğu için korunur)
//   Ctrl+D  açık/koyu mod (Görünüm ayarlarına kaydedilir)
//   F11 ya da sistemin tam ekran kısayolu (macOS'ta Ctrl+Cmd+F)  tam ekran
//   Ctrl+Q  kapat
//
// Geliştirici seçenekleri:
//   --dark               koyu modda aç
//   --launcher           başlat paneli açık olarak başla
//   --screenshot <dosya> açılıştan kısa süre sonra masaüstünün görüntüsünü PNG olarak kaydet ve çık.
//                        Görüntü pencere boyutundan bağımsız olarak 1440x900 çizilir.
//   --size <GxY>         ekran görüntüsünün boyutu, ör. --size 1024x720
//   --window             masaüstünde sahte bir uygulama penceresi göster (pencere tasarımı için)
//   --switcher           Alt+Tab pencere geçişini göster (sahte pencereyle)
//   --all-apps           başlat panelini bütün uygulamalar görünümünde aç
//   --search <metin>     başlat panelini bu aramayla aç, ör. --search koyu
//   --calendar           takvimi açık göster
//   --quick-settings     hızlı ayarları açık göster
//   --login              giriş ekranını göster (compositor --login gibi). Şifre: ankaros
//   --lock               masaüstünü kilitli olarak başla. Şifre: ankaros
//   --error              giriş ya da kilit ekranını "Şifre yanlış" durumunda göster
pragma ComponentBehavior: Bound

import QtQuick
import Ankaros.Shell
import Ankaros.System
import Ankaros.UI

Window {
    id: window

    readonly property list<string> args: Qt.application.arguments
    readonly property bool screenshotMode: args.includes("--screenshot")
    readonly property var screenshotSize: {
        const i = args.indexOf("--size");
        const parts = i >= 0 ? args[i + 1].split("x").map(Number) : [1440, 900];
        return Qt.size(parts[0], parts[1]);
    }

    width: 1440
    height: 900
    // Alt çubuk ve başlat paneli bu boyutun altında sığmaz.
    minimumWidth: 1024
    minimumHeight: 720
    visible: true
    title: qsTr("ankaros: kabuk önizlemesi")
    color: "black"

    // --dark ekran görüntüleri içindir, kullanıcının kayıtlı ayarını değiştirmez.
    Component.onCompleted: {
        if (args.includes("--dark"))
            Theme.dark = true;
    }

    // Ekran görüntüsünün alındığı öğe: masaüstü ya da giriş ekranı
    Item {
        id: screen

        // Ekran görüntüsü modunda pencere ekrana sığmak için küçülse bile görüntü aynı boyutta çıksın.
        width: window.screenshotMode ? window.screenshotSize.width : window.width
        height: window.screenshotMode ? window.screenshotSize.height : window.height

    Desktop {
        id: desktop

        anchors.fill: parent
        visible: !window.args.includes("--login")
        session: Session
        network: Network
        battery: Battery
        locked: window.args.includes("--lock")
        startMenuOpen: window.args.includes("--launcher")
        calendarOpen: window.args.includes("--calendar")
        quickSettingsOpen: window.args.includes("--quick-settings")

        // Sahte veriler. ankaros'ta compositor ve sistem servislerinden gelecek.
        apps: ["Terminal", "Dosyalar", "Tarayıcı", "Ayarlar", "Metin Düzenleyici", "Müzik",
            "Fotoğraflar", "Takvim", "Hesap Makinesi", "Sistem İzleyici", "Notlar", "Paket Merkezi"]
        taskBarApps: ["Dosyalar", "Terminal", "Tarayıcı", "Metin Düzenleyici", "Müzik", "Fotoğraflar"]
        running: ["Dosyalar", "Terminal"]
        activeApp: "Dosyalar"
        recentApps: [
            { app: "Terminal", time: new Date() },
            { app: "Ayarlar", time: new Date(Date.now() - 12 * 60000) },
            { app: "Tarayıcı", time: new Date(Date.now() - 26 * 3600000) },
            { app: "Metin Düzenleyici", time: new Date(Date.now() - 50 * 3600000) }
        ]
        userName: Session.userName
        switcherWindows: window.args.includes("--switcher") ? [
            { app: "Terminal", title: "ankaros@ankaros: ~", item: null, minimized: true },
            { app: "Dosyalar", title: qsTr("Ev"), item: fakeWindow, minimized: false }
        ] : []
        switcherIndex: window.args.includes("--switcher") ? 1 : -1
        onAppLaunched: app => console.log("Başlatılacak uygulama:", app)
        onSettingsRequested: page => console.log("Ayarlar açılacak:", page)

        Component.onCompleted: {
            const search = window.args.indexOf("--search");
            if (search >= 0)
                showStartMenu("home", window.args[search + 1]);
            else if (window.args.includes("--all-apps"))
                showStartMenu("all", "");
        }
    }

    Loader {
        anchors.fill: parent
        active: window.args.includes("--login")

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

                function onLoggedIn() {
                    console.log("Önizleme: oturum açılacaktı");
                    login.busy = false;
                }
            }

            Component.onCompleted: {
                if (window.args.includes("--error"))
                    error = qsTr("Şifre yanlış");
            }
        }
    }
    }

    // Sahte bir uygulama penceresi. ankaros'ta pencereler compositor'dan gelir ve içerikleri
    // gerçek uygulamalardır. Burada sadece pencere çerçevesinin tasarımı denenir.
    WindowChrome {
        id: fakeWindow

        parent: desktop.windowLayer
        visible: window.args.includes("--window") || window.args.includes("--switcher")
        x: 300
        y: 90
        width: 840
        height: 560
        app: "Dosyalar"
        title: "Ev"
        onCloseClicked: visible = false

        Rectangle {
            anchors.fill: parent
            color: Theme.color.surfaceWindow

            Label {
                anchors.centerIn: parent
                text: qsTr("Uygulama içeriği")
                color: Theme.color.textSecondary
            }
        }
    }

    Shortcut {
        sequence: "Ctrl+R"
        onActivated: Qt.exit(42)
    }

    Shortcut {
        sequence: "Ctrl+D"
        onActivated: Appearance.darkMode = !Appearance.darkMode
    }

    Shortcut {
        sequences: [StandardKey.FullScreen, "F11"]
        onActivated: window.visibility = window.visibility === Window.FullScreen ? Window.Windowed
                                                                                 : Window.FullScreen
    }

    Shortcut {
        sequences: [StandardKey.Quit]
        onActivated: Qt.quit()
    }

    Timer {
        // Açılış animasyonlarının bitmesi için kısa bir bekleme.
        running: window.screenshotMode
        interval: 800
        onTriggered: {
            const path = window.args[window.args.indexOf("--screenshot") + 1];
            screen.grabToImage(result => {
                result.saveToFile(path);
                Qt.quit();
            });
        }
    }
}
