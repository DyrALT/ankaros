// Ayarlar › Ağ: kablolu bağlantının durumu ve IP ayarı (Figma: "Faz 2 · Oturum ve Ağ").
// Ayarı ağ servisi (ankaros-network) uygular, bu sayfa network nesnesini çağırır (bkz. Desktop.network).
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

SettingsPage {
    id: root

    property var network: null
    readonly property bool available: network !== null && network.available
    // Seçili yöntem. Ağ servisindeki ayardan başlar, Uygula'ya basılana kadar yalnızca bu sayfada değişir.
    property string mode: available ? network.mode : "dhcp"
    readonly property bool manual: mode === "static"
    readonly property bool dirty: available && (mode !== network.mode
        || (manual && (root.addressField.text !== network.configAddress
                       || root.gatewayField.text !== network.configGateway
                       || root.dnsField.text !== network.configDns)))
    property string message
    readonly property InputField addressField: addressRow.field
    readonly property InputField gatewayField: gatewayRow.field
    readonly property InputField dnsField: dnsRow.field

    title: qsTr("Ağ")

    function reset() {
        root.message = "";
        root.mode = Qt.binding(() => root.available ? root.network.mode : "dhcp");
        root.addressField.text = root.network ? root.network.configAddress : "";
        root.gatewayField.text = root.network ? root.network.configGateway : "";
        root.dnsField.text = root.network ? root.network.configDns : "";
    }

    function apply() {
        root.message = "";
        root.network.apply(root.mode, root.addressField.text.trim(), root.gatewayField.text.trim(), root.dnsField.text.trim());
    }

    component InfoRow: RowLayout {
        property alias label: rowLabel.text
        property alias value: rowValue.text

        Layout.fillWidth: true
        spacing: Theme.spacing.sm

        Label {
            id: rowLabel

            Layout.preferredWidth: 120
            color: Theme.color.textSecondary
            font.pixelSize: Theme.font.small
        }

        Label {
            id: rowValue

            Layout.fillWidth: true
            font.pixelSize: Theme.font.small
            font.features: { "tnum": 1 }
            elide: Text.ElideRight
        }
    }

    component FieldRow: RowLayout {
        property alias label: fieldLabel.text
        property alias field: rowField

        spacing: Theme.spacing.lg

        Label {
            id: fieldLabel

            Layout.preferredWidth: 120
            font.weight: Font.Medium
            opacity: rowField.enabled ? 1 : 0.55
        }

        InputField {
            id: rowField

            enabled: root.manual
        }
    }

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: card.implicitHeight + 28
        radius: 14
        color: Theme.color.surfaceHover

        ColumnLayout {
            id: card

            anchors.fill: parent
            anchors.margins: 14
            spacing: 12

            RowLayout {
                spacing: 12

                Rectangle {
                    implicitWidth: 40
                    implicitHeight: 40
                    radius: 10
                    color: Theme.color.surfaceWindow

                    Icon {
                        anchors.centerIn: parent
                        name: "network"
                        size: 20
                    }
                }

                Column {
                    spacing: 3

                    Label {
                        text: qsTr("Kablolu (%1)").arg(root.available ? root.network.interfaceName : "eth0")
                        font.pixelSize: Theme.font.large
                        font.weight: Font.DemiBold
                    }

                    Row {
                        spacing: 6

                        Rectangle {
                            anchors.verticalCenter: parent.verticalCenter
                            width: 8
                            height: 8
                            radius: 4
                            color: root.available && root.network.connected ? Theme.color.windowMaximize
                                                                            : Theme.color.windowButtonInactive
                        }

                        Label {
                            text: !root.available ? qsTr("Ağ servisine ulaşılamıyor")
                                : !root.network.carrier ? qsTr("Kablo takılı değil")
                                : !root.network.connected ? qsTr("Bağlanıyor")
                                : root.network.mode === "static" ? qsTr("Bağlı · elle ayarlandı")
                                : qsTr("Bağlı · DHCP")
                            color: Theme.color.textSecondary
                            font.pixelSize: Theme.font.caption
                            font.weight: Font.Medium
                        }
                    }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 1
                color: Theme.color.border
            }

            InfoRow {
                label: qsTr("IP adresi")
                value: root.available && root.network.address ? root.network.address : "-"
            }

            InfoRow {
                label: qsTr("Ağ geçidi")
                value: root.available && root.network.gateway ? root.network.gateway : "-"
            }

            InfoRow {
                label: qsTr("DNS")
                value: root.available && root.network.dns ? root.network.dns : "-"
            }

            InfoRow {
                label: qsTr("MAC adresi")
                value: root.available ? root.network.mac : "-"
            }
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 10

        SectionTitle {
            text: qsTr("IP ayarı")
        }

        RowLayout {
            Layout.fillWidth: true

            SettingLabel {
                Layout.fillWidth: true
                text: qsTr("Yöntem")
                info: qsTr("Otomatik: adres, ağ geçidi ve DNS ağdaki DHCP sunucusundan alınır. Elle: bunları siz girersiniz.")
            }

            SegmentedControl {
                options: [{ value: "dhcp", label: qsTr("Otomatik (DHCP)") }, { value: "static", label: qsTr("Elle") }]
                value: root.mode
                segmentWidth: 64
                onSelected: value => {
                    root.message = "";
                    root.mode = value;
                    // Elle'ye geçince alanlar o anki adreslerle dolar, kullanıcı yalnızca değiştirir
                    if (value === "static" && root.addressField.text === "" && root.available) {
                        root.addressField.text = root.network.address;
                        root.gatewayField.text = root.network.gateway;
                        root.dnsField.text = root.network.dns;
                    }
                }
            }
        }

        FieldRow {
            id: addressRow

            label: qsTr("Adres")
        }

        FieldRow {
            id: gatewayRow

            label: qsTr("Ağ geçidi")
        }

        FieldRow {
            id: dnsRow

            label: qsTr("DNS")
        }

        RowLayout {
            visible: root.dirty || root.message.length > 0
            spacing: Theme.spacing.sm

            Item {
                Layout.preferredWidth: 120 + Theme.spacing.lg - Theme.spacing.sm
            }

            PushButton {
                visible: root.dirty
                text: qsTr("Uygula")
                onClicked: root.apply()
            }

            PushButton {
                visible: root.dirty
                primary: false
                text: qsTr("Vazgeç")
                onClicked: root.reset()
            }

            Label {
                visible: root.message.length > 0
                text: root.message
                color: Theme.color.windowClose
                font.weight: Font.Medium
            }
        }
    }

    // DHCP seçiliyken alanlar o anki değerleri soluk olarak gösterir
    Binding {
        target: root.addressField
        property: "text"
        value: root.available ? root.network.address : ""
        when: !root.manual
    }

    Binding {
        target: root.gatewayField
        property: "text"
        value: root.available ? root.network.gateway : ""
        when: !root.manual
    }

    Binding {
        target: root.dnsField
        property: "text"
        value: root.available ? root.network.dns : ""
        when: !root.manual
    }

    Connections {
        target: root.network
        ignoreUnknownSignals: true

        function onApplied() {
            root.reset();
        }

        function onApplyFailed(message: string) {
            root.message = message;
        }
    }

    Component.onCompleted: {
        if (manual)
            reset();
    }
}
