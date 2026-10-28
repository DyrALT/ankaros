// Ayarlar › Kullanıcı: hesap, otomatik giriş ve şifre değiştirme (Figma: "Faz 2 · Oturum ve Ağ").
// İşlemleri oturum yöneticisi yapar, bu sayfa session nesnesini çağırır ve sonucunu gösterir.
import QtQuick
import QtQuick.Layouts

SettingsPage {
    id: root

    property var session: null
    readonly property string userName: session ? session.userName : ""
    // Şifre değiştirmenin sonucu: başarılıysa ok true, mesaj boş değilse gösterilir
    property string message
    property bool ok: false

    readonly property InputField currentPassword: currentRow.field
    readonly property InputField newPassword: newRow.field
    readonly property InputField repeatPassword: repeatRow.field

    title: qsTr("Kullanıcı")

    component PasswordRow: RowLayout {
        property alias label: rowLabel.text
        property alias field: rowField

        spacing: Theme.spacing.lg

        Label {
            id: rowLabel

            Layout.preferredWidth: 140
            font.weight: Font.Medium
        }

        InputField {
            id: rowField

            password: true
        }
    }

    function changePassword() {
        if (root.newPassword.text !== root.repeatPassword.text) {
            root.ok = false;
            root.message = qsTr("Yeni şifreler aynı değil");
            root.repeatPassword.error = true;
            return;
        }
        root.message = "";
        root.session.changePassword(root.currentPassword.text, root.newPassword.text);
    }

    Rectangle {
        Layout.fillWidth: true
        implicitHeight: account.implicitHeight + 28
        radius: 14
        color: Theme.color.surfaceHover

        RowLayout {
            id: account

            anchors.fill: parent
            anchors.margins: 14
            spacing: 14

            Avatar {
                name: root.userName
                size: 56
            }

            Column {
                Layout.fillWidth: true
                spacing: 3

                Label {
                    text: root.userName
                    font.pixelSize: 15
                    font.weight: Font.DemiBold
                }

                Label {
                    text: qsTr("Standart hesap")
                    color: Theme.color.textSecondary
                    font.pixelSize: Theme.font.caption
                }
            }
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 12

        SectionTitle {
            text: qsTr("Giriş")
        }

        RowLayout {
            Layout.fillWidth: true

            SettingLabel {
                Layout.fillWidth: true
                text: qsTr("Otomatik giriş")
                info: qsTr("Açılışta şifre sorulmadan bu hesapla oturum açılır. Ekran kilidi yine şifre ister.")
            }

            ToggleSwitch {
                checked: root.session ? root.session.autologin : false
                onToggled: checked => root.session.setAutologin(checked)
            }
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 10

        SettingLabel {
            text: qsTr("Şifreyi değiştir")
            font.weight: Font.DemiBold
            info: qsTr("Şifreyi yalnızca oturum yöneticisi doğrular, uygulamalar göremez.")
        }

        PasswordRow {
            id: currentRow

            label: qsTr("Mevcut şifre")
        }

        PasswordRow {
            id: newRow

            label: qsTr("Yeni şifre")
        }

        PasswordRow {
            id: repeatRow

            label: qsTr("Yeni şifre (tekrar)")
            field.onAccepted: root.changePassword()
        }

        RowLayout {
            spacing: Theme.spacing.md

            Item {
                Layout.preferredWidth: 140 - Theme.spacing.md + Theme.spacing.lg
            }

            PushButton {
                text: qsTr("Şifreyi değiştir")
                enabled: root.session !== null && root.currentPassword.text.length > 0
                         && root.newPassword.text.length > 0 && root.repeatPassword.text.length > 0
                onClicked: root.changePassword()
            }

            Label {
                visible: root.message.length > 0
                text: root.message
                color: root.ok ? Theme.color.textSecondary : Theme.color.windowClose
                font.weight: Font.Medium
            }
        }
    }

    Connections {
        target: root.session
        ignoreUnknownSignals: true

        function onPasswordUpdated() {
            root.ok = true;
            root.message = qsTr("Şifre değiştirildi");
            root.currentPassword.clear();
            root.newPassword.clear();
            root.repeatPassword.clear();
        }

        function onPasswordChangeFailed(message: string) {
            root.ok = false;
            root.message = message;
            root.currentPassword.error = message.indexOf("Mevcut") >= 0;
        }
    }
}
