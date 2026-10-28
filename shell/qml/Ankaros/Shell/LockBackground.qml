// Giriş ve kilit ekranının arka planı: verilen öğenin (duvar kağıdı ya da masaüstü) bulanık ve
// karartılmış görüntüsü. Görüntü capture() çağrıldığında bir kez alınır ve bulanık hali önbellekte
// tutulur: her karede yeniden bulanıklaştırmak GPU'suz ortamda (softpipe) çok yavaştır. Kilitliyken
// arkadaki uygulamaların değişen içeriği de böylece görünmez.
import QtQuick
import QtQuick.Effects

Item {
    id: root

    property Item source
    property real dim: 0.2

    function capture() {
        snapshot.scheduleUpdate();
    }

    ShaderEffectSource {
        id: snapshot

        anchors.fill: parent
        sourceItem: root.source
        live: false
        visible: false
    }

    Item {
        anchors.fill: parent
        // Bulanık sonuç bir dokuda tutulur, snapshot değişmedikçe yeniden hesaplanmaz
        layer.enabled: true

        MultiEffect {
            anchors.fill: parent
            source: snapshot
            autoPaddingEnabled: false
            blurEnabled: true
            blur: 1
            blurMax: 64
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0.08, 0.05, 0.05, root.dim)
    }
}
