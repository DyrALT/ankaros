// Giriş ekranı (Figma: "Faz 2 · Oturum ve Ağ" > Giriş ekranı). Compositor --login ile çalışırken
// ekranın kendisidir. Şifreyi oturum yöneticisi doğrular, bu bileşen yalnızca submitted sinyalini
// verir ve sonucu error ile alır.
import QtQuick

import Ankaros.UI

Item {
    id: root

    property string userName
    // Doğrulama hatası ("Şifre yanlış"). Boşsa hata yok.
    property string error
    property bool busy: false

    signal submitted(string password)
    signal rebootRequested
    signal poweroffRequested

    function fail(message) {
        root.error = message;
        root.busy = false;
        field.clear();
        field.shake();
        field.focusInput();
    }

    Wallpaper {
        id: wallpaper

        anchors.fill: parent
        visible: false
    }

    LockBackground {
        id: background

        anchors.fill: parent
        source: wallpaper
        dim: 0.2
    }

    Column {
        anchors.centerIn: parent
        anchors.verticalCenterOffset: -20
        spacing: 0

        BigClock {
            anchors.horizontalCenter: parent.horizontalCenter
            size: 96
        }

        Item {
            width: 1
            height: 88
        }

        Avatar {
            anchors.horizontalCenter: parent.horizontalCenter
            name: root.userName
            size: 104
            ring: true
        }

        Label {
            anchors.horizontalCenter: parent.horizontalCenter
            topPadding: 16
            bottomPadding: 22
            text: root.userName
            color: "#ffffff"
            font.pixelSize: 24
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

    GlassIconButton {
        anchors.left: parent.left
        anchors.bottom: parent.bottom
        anchors.margins: 32
        // Kullanıcının klavye düzeni (oturum yöneticisi ANKAROS_KEYBOARD_LAYOUT ile iletir)
        label: ({ "tr-f": "F", "us": "US" })[Appearance.keyboardLayout] ?? "TR"
        tooltip: ({ "tr-f": qsTr("Türkçe F klavye"), "us": qsTr("İngilizce (ABD) klavye") })[Appearance.keyboardLayout]
                 ?? qsTr("Türkçe Q klavye")
    }

    Row {
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: 32
        spacing: 12

        GlassIconButton {
            icon: "restart"
            tooltip: qsTr("Yeniden başlat")
            onClicked: root.rebootRequested()
        }

        GlassIconButton {
            icon: "power"
            tooltip: qsTr("Kapat")
            onClicked: root.poweroffRequested()
        }
    }

    Component.onCompleted: {
        background.capture();
        field.focusInput();
    }
}
