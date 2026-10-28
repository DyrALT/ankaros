// Alt+Tab ile açılan pencere geçişi: açık pencerelerin küçük görüntüleri, son kullanılandan başlayarak.
// Pencerelerin kendisi compositor'da, görüntüleri ShaderEffectSource ile canlı çizilir.
pragma ComponentBehavior: Bound

import QtQuick

import Ankaros.UI

Item {
    id: root

    // [{ app, title, item, minimized }, ...]: item penceresinin kendisi (görüntüsü alınır)
    property var windows: []
    // Seçili pencere. -1 ise geçiş kapalı.
    property int currentIndex: -1
    property Item backdrop
    readonly property bool shown: currentIndex >= 0 && windows.length > 0

    // Bir pencereye tıklandı
    signal activated(int index)

    visible: opacity > 0
    opacity: shown ? 1 : 0

    Behavior on opacity {
        NumberAnimation { duration: Theme.motion.fast }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.color.scrim
        opacity: 0.5
    }

    GlassPanel {
        id: panel

        readonly property real thumbWidth: 220
        readonly property real thumbHeight: 140
        readonly property real itemWidth: thumbWidth + 20

        anchors.centerIn: parent
        anchors.verticalCenterOffset: -30
        width: Math.min(row.implicitWidth, root.width - 2 * Theme.spacing.xxl) + 2 * Theme.spacing.lg
        height: row.implicitHeight + 2 * Theme.spacing.lg
        radius: Theme.radius.lg
        backdrop: root.backdrop
        clip: true

        Row {
            id: row

            // Pencereler sığmazsa seçili olan görünecek kadar kaydırılır
            x: Theme.spacing.lg - Math.max(0, (root.currentIndex + 1) * (panel.itemWidth + spacing) - spacing
                                           - (panel.width - 2 * Theme.spacing.lg))
            y: Theme.spacing.lg
            spacing: Theme.spacing.md

            Repeater {
                id: repeater

                model: root.shown ? root.windows : []

                Rectangle {
                    id: entry

                    required property var modelData
                    required property int index
                    readonly property bool selected: index === root.currentIndex

                    width: panel.itemWidth
                    height: column.implicitHeight + 20
                    radius: 14
                    color: selected ? Theme.color.accentSoft : hover.hovered ? Theme.color.surfaceHover : "transparent"
                    border.width: selected ? 2 : 0
                    border.color: Theme.color.accent

                    HoverHandler {
                        id: hover
                    }

                    TapHandler {
                        onTapped: root.activated(entry.index)
                    }

                    Column {
                        id: column

                        x: 10
                        y: 10
                        spacing: 10

                        Item {
                            width: panel.thumbWidth
                            height: panel.thumbHeight

                            // Pencerenin oranı korunarak alana sığdırılır
                            ShaderEffectSource {
                                readonly property Item window: entry.modelData.item
                                readonly property real fit: window ? Math.min(parent.width / window.width,
                                                                                parent.height / window.height) : 0

                                anchors.centerIn: parent
                                width: window ? window.width * fit : 0
                                height: window ? window.height * fit : 0
                                visible: window !== null && !entry.modelData.minimized
                                sourceItem: visible ? window : null
                                live: true
                                smooth: true
                            }

                            // Küçültülmüş pencere çizilmediği için yerine uygulamanın simgesi
                            AppIcon {
                                anchors.centerIn: parent
                                visible: entry.modelData.minimized
                                app: entry.modelData.app
                                size: 56
                            }
                        }

                        Row {
                            spacing: Theme.spacing.sm

                            AppIcon {
                                anchors.verticalCenter: parent.verticalCenter
                                app: entry.modelData.app
                                size: 18
                            }

                            Label {
                                anchors.verticalCenter: parent.verticalCenter
                                width: panel.thumbWidth - 18 - Theme.spacing.sm
                                text: entry.modelData.title
                                font.pixelSize: Theme.font.small
                                font.weight: entry.selected ? Font.DemiBold : Font.Normal
                                elide: Text.ElideRight
                            }
                        }
                    }
                }
            }
        }
    }
}
