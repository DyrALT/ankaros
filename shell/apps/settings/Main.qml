// Ayarlar uygulamasının penceresi. Pencere çerçevesini ankaros'ta compositor çizer (karar 0007).
import QtQuick

import Ankaros.System
import Ankaros.UI

Window {
    id: window

    // Açılış sayfası: appearance, network, user (bkz. SettingsPanel.pages)
    property string page: "appearance"
    // Boş değilse açılıştan kısa süre sonra pencerenin görüntüsü bu dosyaya kaydedilir ve uygulama kapanır
    property string screenshot
    // Compositor, Ayarlar zaten açıkken başka bir sayfa isterse (başlat panelinde bir ayar arandığında)
    // shell.conf'a yazar ("sayfa|zaman"). Açılıştaki eski istek dikkate alınmaz, yalnızca değişiklik.
    readonly property string settingsRequest: Appearance.settingsRequest

    width: 820
    height: 600
    minimumWidth: 700
    minimumHeight: 480
    visible: true
    title: qsTr("Ayarlar")
    color: Theme.color.surfaceWindow
    onSettingsRequestChanged: {
        const page = settingsRequest.split("|")[0];
        if (page.length > 0)
            panel.page = page;
    }

    SettingsPanel {
        id: panel

        anchors.fill: parent
        page: window.page
        session: Session
        network: Network
    }

    Timer {
        running: window.screenshot.length > 0
        interval: 800
        onTriggered: panel.grabToImage(result => {
            result.saveToFile(window.screenshot);
            Qt.quit();
        })
    }
}
