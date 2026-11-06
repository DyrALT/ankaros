// Mac önizlemesi için sahte ses (Ankaros.System). ankaros'ta bunun yerine C++ Audio sınıfı (audio.cpp)
// wpctl üzerinden varsayılan çıkışı okur ve değiştirir. Önizlemede ses aygıtı varmış gibi davranılır,
// böylece kaydırıcı ve sessiz düğmesi denenebilir.
pragma Singleton

import QtQuick

QtObject {
    readonly property bool available: true
    property int volume: 60
    property bool muted: false

    // C++ Audio'daki (wpctl.h, iconName) kademelerle aynı
    readonly property string icon: muted || volume <= 0 ? "volumeMute"
        : volume < 34 ? "volume1" : volume < 67 ? "volume2" : "volume3"

    function toggleMuted() {
        muted = !muted
    }
}
