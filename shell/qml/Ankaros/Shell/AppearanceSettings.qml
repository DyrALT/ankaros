// Ayarlar › Görünüm: tema, pencere düğmelerinin konumu ve stili. Seçimler ShellSettings'e yazılır,
// açık pencerelere hemen uygulanır. Figma: "B · Pencereler ve Ayarlar" sayfası.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

SettingsPage {
    id: root

    title: qsTr("Görünüm")

    component Radio: Rectangle {
        id: radio

        property bool checked

        implicitWidth: 16
        implicitHeight: 16
        radius: 8
        color: radio.checked ? "#ffffff" : "transparent"
        border.color: radio.checked ? Theme.color.accent : Qt.alpha(Theme.color.textSecondary, 0.6)
        border.width: radio.checked ? 5 : 1.5
    }

    // Önizlemeli seçenek kartı. Önizleme çocuk olarak verilir.
    component OptionCard: Rectangle {
        id: card

        property string label
        property bool selected
        default property alias preview: previewSlot.data

        signal chosen

        implicitWidth: column.implicitWidth + 20
        implicitHeight: column.implicitHeight + 22
        radius: 14
        color: selected ? Theme.color.accentSoft : Theme.color.surfaceHover
        border.color: selected ? Theme.color.accent : Theme.color.border
        border.width: selected ? 2 : 1

        Behavior on color {
            ColorAnimation { duration: Theme.motion.fast }
        }

        Column {
            id: column

            x: 10
            y: 10
            spacing: 10

            Item {
                id: previewSlot

                implicitWidth: childrenRect.width
                implicitHeight: childrenRect.height
                width: implicitWidth
                height: implicitHeight
            }

            Row {
                spacing: Theme.spacing.sm

                Radio {
                    anchors.verticalCenter: parent.verticalCenter
                    checked: card.selected
                }

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: card.label
                    font.weight: card.selected ? Font.DemiBold : Font.Medium
                }
            }
        }

        TapHandler {
            onTapped: card.chosen()
        }
    }

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
                    ShellSettings.darkMode = false;
                    Theme.dark = Qt.binding(() => ShellSettings.darkMode);
                }

                ThemePreview {
                    dark: false
                }
            }

            OptionCard {
                label: qsTr("Koyu")
                selected: Theme.dark
                onChosen: {
                    ShellSettings.darkMode = true;
                    Theme.dark = Qt.binding(() => ShellSettings.darkMode);
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
                value: ShellSettings.windowButtonSide
                onSelected: value => ShellSettings.windowButtonSide = value
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
                    selected: ShellSettings.windowButtonStyle === modelData.value
                    onChosen: ShellSettings.windowButtonStyle = modelData.value

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
