// Mac önizlemesi için sahte sistem bilgisi (Ankaros.System). ankaros'ta bunun yerine C++ SystemInfo
// sınıfı (systeminfo.cpp) değerleri sistemden okur.
pragma Singleton

import QtQuick

QtObject {
    readonly property string version: "0.4 · Faz 3"
    readonly property string kernel: "Linux 6.18.7 (aarch64)"
    readonly property string processor: qsTr("4 çekirdek")
    readonly property string memory: "2 GB"
    readonly property string disk: qsTr("768 MB, %42 dolu")
    readonly property string graphics: "virgl (Apple M2 Pro)"
    readonly property string bootTime: "1,1 sn"

    function copy(text) {
        console.log("Önizleme: panoya kopyalanacaktı:\n" + text);
    }
}
