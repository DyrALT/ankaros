// Önizleme penceresi. Açılış boyutu Figma'daki masaüstü çerçeveleriyle aynıdır (1440x900),
// pencere yeniden boyutlandırılabilir ve tam ekran yapılabilir. Kabuk her boyuta uyum sağlar.
//
// Kısayollar (macOS'ta Ctrl yerine Cmd):
//   Ctrl+R  yeniden derleyip yeniden aç: uygulama 42 (açık mod) ya da 43 (koyu mod) koduyla çıkar,
//           scripts/shell-preview.sh bunu görünce derleyip uygulamayı yeniden başlatır
//   Ctrl+D  açık/koyu mod
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
import QtQuick
import Ankaros.Shell

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

    Component.onCompleted: Theme.dark = args.includes("--dark")

    Desktop {
        id: desktop

        // Ekran görüntüsü modunda pencere ekrana sığmak için küçülse bile görüntü aynı boyutta çıksın.
        width: window.screenshotMode ? window.screenshotSize.width : window.width
        height: window.screenshotMode ? window.screenshotSize.height : window.height
        startMenuOpen: window.args.includes("--launcher")

        // Sahte veriler. ankaros'ta compositor ve sistem servislerinden gelecek.
        apps: ["Terminal", "Dosyalar", "Tarayıcı", "Ayarlar", "Metin Düzenleyici", "Müzik",
            "Fotoğraflar", "Takvim", "Hesap Makinesi", "Sistem İzleyici", "Notlar", "Paket Merkezi"]
        taskBarApps: ["Dosyalar", "Terminal", "Tarayıcı", "Metin Düzenleyici", "Müzik", "Fotoğraflar"]
        running: ["Dosyalar", "Terminal"]
        activeApp: "Dosyalar"
        recommended: [
            { app: "Metin Düzenleyici", title: "rapor.md", subtitle: qsTr("2 dakika önce") },
            { app: "Dosyalar", title: "ankaros", subtitle: qsTr("Dün") },
            { app: "Terminal", title: "build.sh", subtitle: qsTr("Dün") },
            { app: "Takvim", title: "Ekim 2026", subtitle: qsTr("3 gün önce") }
        ]
        userName: "Diyar"
        onAppLaunched: app => console.log("Başlatılacak uygulama:", app)
    }

    // Sahte bir uygulama penceresi. ankaros'ta pencereler compositor'dan gelir ve içerikleri
    // gerçek uygulamalardır. Burada sadece pencere çerçevesinin tasarımı denenir.
    WindowChrome {
        parent: desktop.windowLayer
        visible: window.args.includes("--window")
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
        onActivated: Qt.exit(Theme.dark ? 43 : 42)
    }

    Shortcut {
        sequence: "Ctrl+D"
        onActivated: Theme.dark = !Theme.dark
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
            desktop.grabToImage(result => {
                result.saveToFile(path);
                Qt.quit();
            });
        }
    }
}
