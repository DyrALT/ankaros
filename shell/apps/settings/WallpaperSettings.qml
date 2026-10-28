// Ayarlar › Duvar kağıdı: dört hazır palet ve kilit/giriş ekranında kullanılması. Seçim Appearance'a
// yazılır, masaüstü hemen değişir. Figma: "Faz 3 · Uygulamalar" sayfası.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Effects
import QtQuick.Layouts

import Ankaros.UI

SettingsPage {
    id: root

    title: qsTr("Duvar kağıdı")

    // Duvar kağıdının köşeleri yuvarlatılmış küçük önizlemesi
    component Thumbnail: Item {
        id: thumbnail

        property string name

        width: 104
        height: 66

        Wallpaper {
            anchors.fill: parent
            name: thumbnail.name
            layer.enabled: true
            layer.effect: MultiEffect {
                maskEnabled: true
                maskSource: mask
                maskThresholdMin: 0.5
                maskSpreadAtMin: 1
            }
        }

        Rectangle {
            id: mask

            anchors.fill: parent
            radius: 8
            visible: false
            layer.enabled: true
        }
    }

    ColumnLayout {
        spacing: 10

        SettingLabel {
            text: qsTr("Duvar kağıdı")
            info: qsTr("Her duvar kağıdının açık ve koyu temada ayrı tonları vardır, tema değişince kendiliğinden geçer.")
        }

        Flow {
            Layout.fillWidth: true
            spacing: Theme.spacing.md

            Repeater {
                model: [{ value: "ankara-tasi", label: qsTr("Ankara Taşı") },
                    { value: "bozkir", label: qsTr("Bozkır") },
                    { value: "kehribar", label: qsTr("Kehribar") },
                    { value: "gece", label: qsTr("Gece") }]

                OptionCard {
                    id: card

                    required property var modelData

                    label: modelData.label
                    selected: Appearance.wallpaper === modelData.value
                    onChosen: Appearance.wallpaper = modelData.value

                    Thumbnail {
                        name: card.modelData.value
                    }
                }
            }
        }
    }

    RowLayout {
        Layout.fillWidth: true

        SettingLabel {
            Layout.fillWidth: true
            text: qsTr("Kilit ve giriş ekranında da kullan")
            info: qsTr("Kapalıyken kilit ve giriş ekranında Ankara Taşı duvar kağıdı görünür.")
        }

        ToggleSwitch {
            checked: Appearance.wallpaperOnLock
            onToggled: checked => Appearance.wallpaperOnLock = checked
        }
    }
}
