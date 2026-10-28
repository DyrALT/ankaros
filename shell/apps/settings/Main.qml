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

    width: 820
    height: 600
    minimumWidth: 700
    minimumHeight: 480
    visible: true
    title: qsTr("Ayarlar")
    color: Theme.color.surfaceWindow

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
