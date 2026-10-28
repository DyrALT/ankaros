// Ayarlar › Görünüm: tema, pencere düğmelerinin konumu ve stili. Seçimler Appearance'a yazılır,
// açık pencerelere hemen uygulanır. Figma: "B · Pencereler ve Ayarlar" sayfası.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Ankaros.UI

SettingsPage {
    id: root

    title: qsTr("Görünüm")

    // Tema kartının küçük önizlemesi: duvar kağıdı, bir pencere ve alt çubuk
    component ThemePreview: Rectangle {
        id: themePreview

        property bool dark

        width: 168
        height: 96
        radius: 8
        clip: true
        gradient: Gradient {
            GradientStop {
                position: 0
                color: themePreview.dark ? "#2b1a16" : "#f4e3dc"
            }
            GradientStop {
                position: 1
                color: themePreview.dark ? "#4a2a2a" : "#d99a84"
            }
        }

        Rectangle {
            x: 36
            y: 12
            width: 96
            height: 52
            radius: 4
            color: themePreview.dark ? "#201e1d" : "#fbfaf8"
        }

        Rectangle {
            x: 8
            y: 78
            width: 152
            height: 12
            radius: 4
            color: themePreview.dark ? Qt.rgba(0.118, 0.110, 0.106, 0.75) : Qt.rgba(1, 1, 1, 0.75)

            Rectangle {
                x: 3
                y: 2
                width: 8
                height: 8
                radius: 2
                color: Theme.color.accent
            }
        }
    }

    ColumnLayout {
        spacing: 10

        SectionTitle {
            text: qsTr("Tema")
        }

        Row {
            spacing: Theme.spacing.md

            OptionCard {
                label: qsTr("Açık")
                // Kartlar o an geçerli temayı gösterir (önizlemenin --dark seçeneği ayarı değiştirmez)
                selected: !Theme.dark
                onChosen: {
                    Appearance.darkMode = false;
                    Theme.dark = Qt.binding(() => Appearance.darkMode);
                }

                ThemePreview {
                    dark: false
                }
            }

            OptionCard {
                label: qsTr("Koyu")
                selected: Theme.dark
                onChosen: {
                    Appearance.darkMode = true;
                    Theme.dark = Qt.binding(() => Appearance.darkMode);
                }

                ThemePreview {
                    dark: true
                }
            }
        }
    }

    ColumnLayout {
        spacing: 14

        SectionTitle {
            text: qsTr("Pencere düğmeleri")
        }

        RowLayout {
            spacing: Theme.spacing.lg

            SettingLabel {
                Layout.preferredWidth: 200
                text: qsTr("Konum")
                info: qsTr("Kapat, küçült ve büyüt düğmelerinin başlık çubuğunun hangi tarafında duracağı. Değişiklik açık pencerelere hemen uygulanır.")
            }

            SegmentedControl {
                options: [{ value: "left", label: qsTr("Sol") }, { value: "right", label: qsTr("Sağ") }]
                value: Appearance.windowButtonSide
                onSelected: value => Appearance.windowButtonSide = value
            }
        }

        Row {
            spacing: Theme.spacing.md

            Repeater {
                model: [{ value: "icons", label: qsTr("Simgeler") },
                    { value: "colored", label: qsTr("Renkli daireler") }]

                OptionCard {
                    id: styleCard

                    required property var modelData

                    label: modelData.label
                    selected: Appearance.windowButtonStyle === modelData.value
                    onChosen: Appearance.windowButtonStyle = modelData.value

                    Rectangle {
                        width: 250
                        height: 46
                        radius: Theme.radius.sm
                        color: Theme.color.surfaceWindow
                        border.color: Theme.color.border
                        clip: true

                        WindowTitleBar {
                            anchors.fill: parent
                            anchors.margins: 1
                            color: "transparent"
                            app: "Dosyalar"
                            title: qsTr("Ev")
                            buttonStyle: styleCard.modelData.value
                        }
                    }
                }
            }
        }
    }
}
