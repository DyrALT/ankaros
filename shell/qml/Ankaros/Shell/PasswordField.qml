// Giriş ve kilit ekranının şifre alanı (Figma: Şifre alanı). Gönder düğmesi alanın içindedir, böylece
// alan ekranda tek başına ortalanır. Hatalı şifrede kırmızı kenarlıkla sallanır.
import QtQuick

Item {
    id: root

    property bool error: false
    property bool busy: false
    readonly property string text: input.text

    signal submitted(string password)
    // Kullanıcı yazmaya başladı (ekran hatayı bu durumda temizler)
    signal edited

    function clear() {
        input.text = "";
    }

    function focusInput() {
        input.forceActiveFocus();
    }

    // Hatalı şifrede kısa bir sallanma
    function shake() {
        shakeAnimation.restart();
    }

    function submit() {
        if (input.text.length > 0 && !root.busy)
            root.submitted(input.text);
    }

    implicitWidth: 280
    implicitHeight: 44

    Rectangle {
        id: frame

        width: parent.width
        height: parent.height
        radius: height / 2
        color: Qt.rgba(1, 1, 1, input.text.length > 0 ? 0.22 : 0.16)
        border.color: root.error ? Theme.color.windowClose
                                 : Qt.rgba(1, 1, 1, input.activeFocus ? 0.7 : 0.3)
        border.width: input.activeFocus || root.error ? 1.5 : 1

        TextInput {
            id: input

            anchors.left: parent.left
            anchors.right: send.left
            anchors.leftMargin: 18
            anchors.rightMargin: 8
            anchors.verticalCenter: parent.verticalCenter
            color: "#ffffff"
            font.family: Theme.font.family
            font.pixelSize: Theme.font.large
            font.weight: Font.DemiBold
            font.letterSpacing: 1.5
            echoMode: TextInput.Password
            passwordCharacter: "•"
            selectionColor: Qt.rgba(1, 1, 1, 0.3)
            clip: true
            onAccepted: root.submit()
            onTextChanged: {
                if (text.length > 0)
                    root.edited();
            }

            Label {
                anchors.verticalCenter: parent.verticalCenter
                visible: input.text.length === 0
                text: qsTr("Şifre")
                color: Qt.rgba(1, 1, 1, 0.7)
                font.pixelSize: Theme.font.large
            }
        }

        Rectangle {
            id: send

            anchors.right: parent.right
            anchors.rightMargin: 5
            anchors.verticalCenter: parent.verticalCenter
            width: 34
            height: 34
            radius: 17
            color: input.text.length > 0 ? Theme.color.accent : Qt.rgba(1, 1, 1, 0.18)

            Behavior on color {
                ColorAnimation { duration: Theme.motion.fast }
            }

            Icon {
                anchors.centerIn: parent
                name: "chevronRight"
                size: 16
                color: "#ffffff"
                opacity: root.busy ? 0.4 : 1
            }

            TapHandler {
                onTapped: root.submit()
            }
        }
    }

    SequentialAnimation {
        id: shakeAnimation

        loops: 1
        NumberAnimation { target: frame; property: "x"; to: -10; duration: 50 }
        NumberAnimation { target: frame; property: "x"; to: 10; duration: 70 }
        NumberAnimation { target: frame; property: "x"; to: -6; duration: 60 }
        NumberAnimation { target: frame; property: "x"; to: 0; duration: 50 }
    }
}
