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
//   --launcher           başlatıcı açık olarak başla
//   --screenshot <dosya> açılıştan kısa süre sonra pencerenin görüntüsünü PNG olarak kaydet ve çık
import QtQuick
import Ankaros.Shell

Window {
    id: window

    readonly property list<string> args: Qt.application.arguments

    color: "black"
    height: 900
    maximumHeight: 900
    maximumWidth: 1440
    minimumHeight: 900
    minimumWidth: 1440
    title: qsTr("ankaros: kabuk önizlemesi")
    visible: true
    width: 1440

    Component.onCompleted: Theme.dark = args.includes("--dark")

    Desktop {
        id: desktop

        anchors.fill: parent
        // Sahte uygulama listesi. ankaros'ta sistemdeki uygulamalardan gelecek.
        apps: [
            {
                name: "Terminal",
                color: "#2d3436"
            },
            {
                name: "Dosyalar",
                color: "#0984e3"
            },
            {
                name: "Tarayıcı",
                color: "#e17055"
            },
            {
                name: "Ayarlar",
                color: "#636e72"
            },
            {
                name: "Metin Düzenleyici",
                color: "#00b894"
            },
            {
                name: "Müzik",
                color: "#e84393"
            },
            {
                name: "Fotoğraflar",
                color: "#fdcb6e"
            },
            {
                name: "Takvim",
                color: "#d63031"
            },
            {
                name: "Hesap Makinesi",
                color: "#6c5ce7"
            },
            {
                name: "Sistem İzleyici",
                color: "#00cec9"
            },
            {
                name: "Notlar",
                color: "#ffeaa7"
            },
            {
                name: "Paket Merkezi",
                color: "#a29bfe"
            }
        ]
        launcherOpen: window.args.includes("--launcher")
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
        interval: 800
        // Açılış animasyonlarının bitmesi için kısa bir bekleme.
        running: window.args.includes("--screenshot")

        onTriggered: {
            const path = window.args[window.args.indexOf("--screenshot") + 1];
            desktop.grabToImage(result => {
                result.saveToFile(path);
                Qt.quit();
            });
        }
    }
}
