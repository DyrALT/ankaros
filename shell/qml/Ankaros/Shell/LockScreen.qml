// Kilit ekranı (Figma: "Faz 2 · Oturum ve Ağ" > Kilit ekranı). Masaüstünün üstünü kaplar, arkasındaki
// her şey bulanıklaşır ve tıklamalar ile tuşlar uygulamalara ulaşmaz.
import QtQuick

Item {
    id: root

    property string userName
    // Bulanıklaştırılacak öğe (masaüstü sahnesi)
    property Item backdrop
    property string error
    property bool busy: false

    signal submitted(string password)

    function fail(message) {
        root.error = message;
        root.busy = false;
        field.clear();
        field.shake();
        field.focusInput();
    }

    function reset() {
        root.error = "";
        root.busy = false;
        field.clear();
        field.focusInput();
    }

    visible: opacity > 0

    Behavior on opacity {
        NumberAnimation { duration: Theme.motion.slow; easing.type: Theme.motion.easing }
    }

    onVisibleChanged: {
        if (visible) {
            background.capture();
            reset();
        }
    }

    // Arkadaki uygulamalara tıklama ve tekerlek olayı geçmesin
    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onWheel: wheel => wheel.accepted = true
    }

    LockBackground {
        id: background

        anchors.fill: parent
        source: root.backdrop
        dim: 0.5
    }

    Column {
        anchors.centerIn: parent
        spacing: 0

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: lockedRow.implicitWidth + 22
            height: 26
            radius: 13
            color: Qt.rgba(1, 1, 1, 0.12)

            Row {
                id: lockedRow

                anchors.centerIn: parent
                spacing: 6

                Icon {
                    anchors.verticalCenter: parent.verticalCenter
                    name: "lock"
                    size: 13
                    color: "#ffffff"
                }

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    text: qsTr("Kilitli")
                    color: "#ffffff"
                    font.pixelSize: Theme.font.small
                    font.weight: Font.DemiBold
                }
            }
        }

        Item {
            width: 1
            height: 28
        }

        BigClock {
            anchors.horizontalCenter: parent.horizontalCenter
            size: 136
        }

        Item {
            width: 1
            height: 96
        }

        Avatar {
            anchors.horizontalCenter: parent.horizontalCenter
            name: root.userName
            size: 64
            ring: true
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            topPadding: 12
            bottomPadding: 18
            text: root.userName
            color: "#ffffff"
            font.pixelSize: 17
            font.weight: Font.DemiBold
        }

        PasswordField {
            id: field

            anchors.horizontalCenter: parent.horizontalCenter
            error: root.error.length > 0
            busy: root.busy
            onEdited: root.error = ""
            onSubmitted: password => {
                root.busy = true;
                root.submitted(password);
            }
        }

        Item {
            width: 1
            height: 10
        }

        ErrorPill {
            anchors.horizontalCenter: parent.horizontalCenter
            text: root.error
        }
    }
}
