// Önizleme penceresi. Boyutu Figma'daki masaüstü çerçeveleriyle aynıdır (1440x900).
//
// Kısayollar (macOS'ta Ctrl yerine Cmd):
//   Ctrl+R  yeniden derleyip yeniden aç: uygulama 42 (açık mod) ya da 43 (koyu mod) koduyla çıkar,
//           scripts/shell-preview.sh bunu görünce derleyip uygulamayı yeniden başlatır
//   Ctrl+D  açık/koyu mod
//   Ctrl+Q  kapat
//
// Geliştirici seçenekleri:
//   --dark               koyu modda aç
//   --launcher           başlat paneli açık olarak başla
//   --screenshot <dosya> açılıştan kısa süre sonra pencerenin görüntüsünü PNG olarak kaydet ve çık
import QtQuick
import Ankaros.Shell

Window {
    id: window

    readonly property list<string> args: Qt.application.arguments

    width: 1440
    height: 900
    minimumWidth: 1440
    maximumWidth: 1440
    minimumHeight: 900
    maximumHeight: 900
    visible: true
    title: qsTr("ankaros: kabuk önizlemesi")
    color: "black"

    Component.onCompleted: Theme.dark = args.includes("--dark")

    Desktop {
        id: desktop

        anchors.fill: parent
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

    Shortcut {
        sequence: "Ctrl+R"
        onActivated: Qt.exit(Theme.dark ? 43 : 42)
    }

    Shortcut {
        sequence: "Ctrl+D"
        onActivated: Theme.dark = !Theme.dark
    }

    Shortcut {
        sequences: [StandardKey.Quit]
        onActivated: Qt.quit()
    }

    Timer {
        // Açılış animasyonlarının bitmesi için kısa bir bekleme.
        running: window.args.includes("--screenshot")
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
