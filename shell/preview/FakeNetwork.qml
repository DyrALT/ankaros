// Önizleme için sahte ağ. ankaros'ta compositor'ın C++ Network nesnesi ağ servisinden (ankaros-network)
// gelen gerçek durumu verir. Burada ayar uygulanınca değerler biraz gecikmeyle değişir.
import QtQuick

QtObject {
    id: root

    readonly property bool available: true
    readonly property string interfaceName: "eth0"
    readonly property bool carrier: true
    readonly property bool connected: true
    property string mode: "dhcp"
    property string configAddress: mode === "static" ? "192.168.1.50/24" : ""
    property string configGateway: mode === "static" ? "192.168.1.1" : ""
    property string configDns: mode === "static" ? "1.1.1.1" : ""
    readonly property string address: mode === "static" ? configAddress : "10.0.2.15/24"
    readonly property string gateway: mode === "static" ? configGateway : "10.0.2.2"
    readonly property string dns: mode === "static" ? configDns : "10.0.2.3"
    readonly property string mac: "52:54:00:12:34:56"

    signal applied
    signal applyFailed(string message)

    function apply(newMode, address, gateway, dns) {
        const timer = Qt.createQmlObject("import QtQuick; Timer { interval: 300 }", root);
        timer.triggered.connect(() => {
            if (newMode === "static" && !/^\d+\.\d+\.\d+\.\d+\/\d+$/.test(address)) {
                root.applyFailed(qsTr("Adres 192.168.1.50/24 biçiminde olmalı"));
            } else {
                root.mode = newMode;
                root.configAddress = address;
                root.configGateway = gateway;
                root.configDns = dns;
                root.applied();
            }
            timer.destroy();
        });
        timer.start();
    }
}
