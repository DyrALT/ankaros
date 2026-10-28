// Pencere başlık çubuğu: uygulama simgesi, başlık ve küçült/büyüt/kapat düğmeleri.
// Compositor tarafından bütün uygulamalar için çizilir (karar 0007). Figma: WindowTitleBar.
// Düğmelerin konumu ve stili kullanıcının Görünüm ayarlarından gelir. Ayarlar penceresindeki
// önizlemeler farklı seçenekleri göstermek için bu iki özelliği doğrudan verir.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts


Rectangle {
    id: root

    property string app
    property string title
    property bool active: true
    // "right" ya da "left"
    property string buttonSide: Appearance.windowButtonSide
    // "icons" ya da "colored"
    property string buttonStyle: Appearance.windowButtonStyle

    signal minimizeClicked
    signal maximizeClicked
    signal closeClicked

    implicitHeight: 46
    color: Theme.color.surfaceWindow

    // Sağdayken küçült, büyüt, kapat. Soldayken kapat, küçült, büyüt (kapat köşede kalır).
    readonly property var buttons: buttonSide === "left" ? ["close", "minimize", "maximize"]
                                                         : ["minimize", "maximize", "close"]

    function clicked(button) {
        if (button === "close")
            closeClicked();
        else if (button === "minimize")
            minimizeClicked();
        else
            maximizeClicked();
    }

    // Çizgi simgeli düğme: üzerine gelince arka planı belirir.
    component IconButton: HoverButton {
        id: iconButton

        property string kind

        implicitWidth: 28
        implicitHeight: 28
        radius: 14
        restColor: kind === "close" ? Theme.color.surfaceHover : "transparent"
        onClicked: root.clicked(kind)

        Icon {
            anchors.centerIn: parent
            name: iconButton.kind
            size: 14
            color: root.active ? Theme.color.textPrimary : Theme.color.textSecondary
        }
    }

    // Renkli daire: odakta değilken gri, üzerine gelince içinde simge belirir.
    component DotButton: Rectangle {
        id: dot

        property string kind
        property bool groupHovered

        implicitWidth: 14
        implicitHeight: 14
        radius: 7
        color: !root.active && !groupHovered ? Theme.color.windowButtonInactive
             : kind === "close" ? Theme.color.windowClose
             : kind === "minimize" ? Theme.color.windowMinimize : Theme.color.windowMaximize
        border.color: Qt.rgba(0, 0, 0, 0.12)
        border.width: 0.5

        Behavior on color {
            ColorAnimation { duration: Theme.motion.fast }
        }

        Icon {
            anchors.centerIn: parent
            name: dot.kind
            size: 9
            color: "#401f14"
            opacity: dot.groupHovered ? 1 : 0

            Behavior on opacity {
                NumberAnimation { duration: Theme.motion.fast }
            }
        }

        TapHandler {
            onTapped: root.clicked(dot.kind)
        }
    }

    component Buttons: RowLayout {
        id: group

        spacing: root.buttonStyle === "colored" ? 8 : 4

        HoverHandler {
            id: groupHover
        }

        Repeater {
            model: root.buttons

            Loader {
                id: slot

                required property string modelData

                sourceComponent: root.buttonStyle === "colored" ? dotComponent : iconComponent

                Component {
                    id: iconComponent

                    IconButton {
                        kind: slot.modelData
                    }
                }

                Component {
                    id: dotComponent

                    DotButton {
                        kind: slot.modelData
                        groupHovered: groupHover.hovered
                    }
                }
            }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: Theme.spacing.lg
        anchors.rightMargin: root.buttonSide === "right" && root.buttonStyle === "icons" ? Theme.spacing.md
                                                                                        : Theme.spacing.lg
        spacing: 10

        Buttons {
            visible: root.buttonSide === "left"
            Layout.rightMargin: Theme.spacing.xs
        }

        AppIcon {
            app: root.app
            size: 20
        }

        Label {
            Layout.fillWidth: true
            text: root.title
            elide: Text.ElideRight
            color: root.active ? Theme.color.textPrimary : Theme.color.textSecondary
            font.weight: Font.DemiBold
        }

        Buttons {
            visible: root.buttonSide === "right"
        }
    }
}
