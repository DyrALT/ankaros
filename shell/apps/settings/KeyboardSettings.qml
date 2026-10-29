// Ayarlar › Klavye ve dil: klavye düzeni ve sistem dili. Seçilen düzen Appearance'a yazılır,
// compositor uygulamalara ve kendi arayüzüne hemen uygular, oturum yöneticisi giriş ekranına da verir.
// Figma: "Faz 3 · Masaüstü cilası" sayfası.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Ankaros.UI

SettingsPage {
    id: root

    // Her düzenin üst harf sırası, düzenler arasındaki farkı göstermek için
    readonly property var layouts: [
        { id: "tr", name: qsTr("Türkçe Q"), keys: "Q W E R T Y U I O P Ğ Ü" },
        { id: "tr-f", name: qsTr("Türkçe F"), keys: "F G Ğ I O D R N H P Q W" },
        { id: "us", name: qsTr("İngilizce (ABD)"), keys: "Q W E R T Y U I O P [ ]" }
    ]

    title: qsTr("Klavye ve dil")

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 6

        SettingLabel {
            text: qsTr("Klavye düzeni")
            info: qsTr("Seçim hemen geçerli olur. Giriş ve kilit ekranında da aynı düzen kullanılır.")
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: column.implicitHeight + 8
            radius: 14
            color: Theme.color.surfaceHover

            Column {
                id: column

                y: 4
                width: parent.width

                Repeater {
                    model: root.layouts

                    HoverButton {
                        id: option

                        required property var modelData
                        readonly property bool selected: Appearance.keyboardLayout === modelData.id

                        width: column.width
                        height: 40
                        radius: Theme.radius.sm
                        onClicked: Appearance.keyboardLayout = modelData.id

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 14
                            anchors.rightMargin: 14
                            spacing: 12

                            Radio {
                                checked: option.selected
                            }

                            Label {
                                Layout.fillWidth: true
                                text: option.modelData.name
                                font.weight: option.selected ? Font.DemiBold : Font.Normal
                            }

                            Label {
                                text: option.modelData.keys
                                color: Theme.color.textSecondary
                                font.pixelSize: Theme.font.small
                                font.letterSpacing: 1
                            }
                        }
                    }
                }
            }
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 12

        SectionTitle {
            text: qsTr("Dil")
        }

        RowLayout {
            Layout.fillWidth: true

            SettingLabel {
                Layout.fillWidth: true
                text: qsTr("Sistem dili")
                info: qsTr("ankaros şimdilik yalnızca Türkçe. Başka diller ileride eklenecek.")
            }

            Label {
                text: qsTr("Türkçe")
                color: Theme.color.textSecondary
            }
        }
    }
}
