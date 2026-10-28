// Ayarlar penceresinin içeriği: kenar çubuğu ve Görünüm bölümü (tema, pencere düğmelerinin konumu
// ve stili). Seçimler ShellSettings'e yazılır, açık pencerelere hemen uygulanır.
// Figma: "B · Pencereler ve Ayarlar" sayfası. Diğer bölümler (Duvar kağıdı, Ağ...) sonraki fazlarda.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    color: Theme.color.surfaceWindow

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

    component SectionTitle: Label {
        font.weight: Font.DemiBold
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: 200
            color: Theme.color.surfaceSidebar

            Column {
                anchors.fill: parent
                anchors.margins: 10
                anchors.topMargin: 8
                spacing: 2

                Repeater {
                    model: [qsTr("Görünüm"), qsTr("Duvar kağıdı"), qsTr("Alt çubuk"), qsTr("Bildirimler"),
                        qsTr("Ağ"), qsTr("Hakkında")]

                    Rectangle {
                        id: item

                        required property string modelData
                        required property int index

                        width: parent.width
                        height: 34
                        radius: Theme.radius.sm
                        color: index === 0 ? Theme.color.accentSoft : "transparent"

                        Label {
                            anchors.verticalCenter: parent.verticalCenter
                            x: 10
                            text: item.modelData
                            color: item.index === 0 ? Theme.color.accent : Theme.color.textPrimary
                            font.weight: item.index === 0 ? Font.DemiBold : Font.Normal
                            // Diğer bölümler henüz yok
                            opacity: item.index === 0 ? 1 : 0.5
                        }
                    }
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.leftMargin: 32
            Layout.rightMargin: 32
            Layout.topMargin: Theme.spacing.xl
            spacing: 22

            Label {
                text: qsTr("Görünüm")
                font.pixelSize: Theme.font.title
                font.weight: Font.Bold
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

                    Column {
                        Layout.preferredWidth: 200
                        spacing: 2

                        Label {
                            text: qsTr("Konum")
                            font.weight: Font.Medium
                        }

                        Label {
                            text: qsTr("Başlık çubuğunun hangi tarafında")
                            color: Theme.color.textSecondary
                            font.pixelSize: Theme.font.caption
                        }
                    }

                    Rectangle {
                        implicitWidth: segments.implicitWidth + 6
                        implicitHeight: 36
                        radius: 10
                        color: Theme.color.surfaceHover

                        Row {
                            id: segments

                            anchors.centerIn: parent
                            spacing: 2

                            Repeater {
                                model: [{ value: "left", label: qsTr("Sol") }, { value: "right", label: qsTr("Sağ") }]

                                Rectangle {
                                    id: segment

                                    required property var modelData
                                    readonly property bool selected: ShellSettings.windowButtonSide === modelData.value

                                    width: 84
                                    height: 30
                                    radius: Theme.radius.sm
                                    color: selected ? Theme.color.surfaceWindow : "transparent"

                                    Behavior on color {
                                        ColorAnimation { duration: Theme.motion.fast }
                                    }

                                    Label {
                                        anchors.centerIn: parent
                                        text: segment.modelData.label
                                        color: segment.selected ? Theme.color.textPrimary : Theme.color.textSecondary
                                        font.weight: segment.selected ? Font.DemiBold : Font.Medium
                                    }

                                    TapHandler {
                                        onTapped: ShellSettings.windowButtonSide = segment.modelData.value
                                    }
                                }
                            }
                        }
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

            Label {
                text: qsTr("Değişiklikler açık pencerelere hemen uygulanır.")
                color: Theme.color.textSecondary
                font.pixelSize: Theme.font.small
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }
}
