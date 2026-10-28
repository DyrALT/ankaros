// Ayarlar › Hakkında: sürüm ve sistem bilgisi (SystemInfo). "Bilgileri kopyala" hepsini metin olarak
// panoya kopyalar, hata bildirirken yapıştırmak için. Figma: "Faz 3 · Uygulamalar" sayfası.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Ankaros.System
import Ankaros.UI

SettingsPage {
    id: root

    readonly property var rows: [
        { label: qsTr("Çekirdek"), value: SystemInfo.kernel },
        { label: qsTr("İşlemci"), value: SystemInfo.processor },
        { label: qsTr("Bellek"), value: SystemInfo.memory },
        { label: qsTr("Disk"), value: SystemInfo.disk },
        { label: qsTr("Ekran kartı"), value: SystemInfo.graphics },
        { label: qsTr("Açılış süresi"), value: SystemInfo.bootTime,
            info: qsTr("Çekirdeğin başlamasından masaüstünün ilk karesine kadar geçen süre (ankarosctl boot).") },
        { label: qsTr("Lisans"), value: "MIT" }
    ]

    title: qsTr("Hakkında")

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: header.implicitHeight + 28
        radius: 14
        color: Theme.color.surfaceHover

        RowLayout {
            id: header

            anchors.fill: parent
            anchors.margins: 14
            spacing: 14

            Rectangle {
                implicitWidth: 56
                implicitHeight: 56
                radius: 13
                color: Theme.color.accent

                Icon {
                    anchors.centerIn: parent
                    name: "logo"
                    size: 30
                    color: Theme.color.textOnAccent
                }
            }

            Column {
                Layout.fillWidth: true
                spacing: 3

                Label {
                    text: "ankaros"
                    font.pixelSize: 18
                    font.weight: Font.Bold
                }

                Label {
                    text: qsTr("Sürüm %1").arg(SystemInfo.version)
                    color: Theme.color.textSecondary
                    font.pixelSize: Theme.font.caption
                }
            }
        }
    }

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: table.implicitHeight + 16
        radius: 14
        color: Theme.color.surfaceHover

        Column {
            id: table

            x: 10
            y: 8
            width: parent.width - 20

            Repeater {
                model: root.rows

                RowLayout {
                    id: row

                    required property var modelData

                    width: table.width
                    height: 23

                    SettingLabel {
                        Layout.preferredWidth: 112
                        text: row.modelData.label
                        info: row.modelData.info ?? ""
                        font.weight: Font.Normal
                        opacity: 0.7
                    }

                    Label {
                        Layout.fillWidth: true
                        text: row.modelData.value.length > 0 ? row.modelData.value : "-"
                        font.weight: Font.Medium
                        elide: Text.ElideRight
                    }
                }
            }
        }
    }

    PushButton {
        primary: false
        text: qsTr("Bilgileri kopyala")
        onClicked: SystemInfo.copy("ankaros " + SystemInfo.version + "\n"
                                   + root.rows.map(row => row.label + ": " + row.value).join("\n"))
    }
}
