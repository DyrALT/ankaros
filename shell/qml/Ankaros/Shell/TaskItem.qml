// Alt çubuktaki uygulama: simge ve altında çalışma göstergesi.
// Gösterge: çalışmıyorsa yok, çalışıyorsa kısa, etkin pencereyse uzun.
import QtQuick

HoverButton {
    id: root

    property string app
    property bool running: false
    property bool active: false

    implicitWidth: Theme.size.taskIcon + 2 * Theme.spacing.xs
    implicitHeight: Theme.size.taskIcon + 11
    radius: 10
    checked: active

    AppIcon {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 5
        app: root.app
        size: Theme.size.taskIcon
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 2
        width: root.active ? 16 : 6
        height: 3
        radius: 1.5
        color: Theme.color.accent
        visible: root.running

        Behavior on width {
            NumberAnimation { duration: Theme.motion.normal; easing.type: Theme.motion.easing }
        }
    }
}
