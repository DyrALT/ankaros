// Başlatıcıdaki tek bir uygulama: simge ve ad.
// Gerçek simgeler gelene kadar uygulama adının baş harfi renkli bir kare üzerinde gösterilir.
import QtQuick

Item {
    id: root

    property string name
    property color tint: Theme.color.accent

    signal clicked

    height: Theme.size.appTile
    width: Theme.size.appTile

    Rectangle {
        anchors.fill: parent
        anchors.margins: Theme.spacing.xs
        color: mouse.pressed ? Theme.color.surfacePressed : mouse.containsMouse
                               ? Theme.color.surfaceHover : "transparent"
        radius: Theme.radius.md

        Behavior on color {
            ColorAnimation {
                duration: Theme.motion.fast
            }
        }
    }
    Column {
        anchors.centerIn: parent
        spacing: Theme.spacing.sm

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            color: root.tint
            height: Theme.size.appIcon
            radius: Theme.radius.lg
            scale: mouse.pressed ? 0.92 : 1
            width: Theme.size.appIcon

            Behavior on scale {
                NumberAnimation {
                    duration: Theme.motion.fast
                    easing.type: Theme.motion.easing
                }
            }

            Text {
                anchors.centerIn: parent
                color: "#ffffff"
                font.pixelSize: Theme.font.title
                font.weight: Font.Bold
                text: root.name.charAt(0)
            }
        }
        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            color: Theme.color.textOnScrim
            elide: Text.ElideRight
            font.pixelSize: Theme.font.small
            horizontalAlignment: Text.AlignHCenter
            text: root.name
            width: Theme.size.appTile - Theme.spacing.md
        }
    }
    MouseArea {
        id: mouse

        anchors.fill: parent
        hoverEnabled: true

        onClicked: root.clicked()
    }
}
