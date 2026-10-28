// Tek satırlık metin alanı (Figma: TextField). Odakta vurgu renginde, hatada kırmızı kenarlıkla çizilir.
import QtQuick

Rectangle {
    id: root

    property alias text: input.text
    property string placeholder
    property bool password: false
    property bool error: false
    readonly property bool focused: input.activeFocus

    signal accepted

    function clear() {
        input.text = "";
    }

    function focusInput() {
        input.forceActiveFocus();
    }

    implicitWidth: 260
    implicitHeight: 36
    radius: 10
    color: enabled ? Theme.color.surfaceWindow : Theme.color.surfaceHover
    opacity: enabled ? 1 : 0.55
    border.color: error ? Theme.color.windowClose : focused ? Theme.color.accent : Theme.color.border
    border.width: error || focused ? 2 : 1

    TextInput {
        id: input

        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 12
        verticalAlignment: TextInput.AlignVCenter
        color: Theme.color.textPrimary
        selectionColor: Theme.color.accentSoft
        selectedTextColor: Theme.color.textPrimary
        font.family: Theme.font.family
        font.pixelSize: Theme.font.body
        echoMode: root.password ? TextInput.Password : TextInput.Normal
        passwordCharacter: "•"
        clip: true
        onAccepted: root.accepted()
        onTextChanged: root.error = false

        Label {
            anchors.verticalCenter: parent.verticalCenter
            visible: input.text.length === 0
            text: root.placeholder
            color: Theme.color.textSecondary
        }
    }
}
