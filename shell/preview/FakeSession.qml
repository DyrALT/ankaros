// Önizleme için sahte oturum. ankaros'ta compositor'ın C++ Session nesnesi oturum yöneticisiyle
// (ankaros-session) konuşur. Burada şifre "ankaros"tur, cevaplar gerçeğe benzesin diye biraz gecikir.
import QtQuick

QtObject {
    id: root

    readonly property string userName: "ankaros"
    property bool autologin: true
    readonly property string password: "ankaros"

    signal unlocked
    signal unlockFailed(string message)
    signal loginFailed(string message)
    signal loggedIn
    signal passwordUpdated
    signal passwordUpdateFailed(string message)

    function later(callback) {
        const timer = Qt.createQmlObject("import QtQuick; Timer { interval: 300 }", root);
        timer.triggered.connect(() => {
            callback();
            timer.destroy();
        });
        timer.start();
    }

    function unlock(password) {
        later(() => password === root.password ? root.unlocked() : root.unlockFailed(qsTr("Şifre yanlış")));
    }

    function login(user, password) {
        later(() => password === root.password ? root.loggedIn() : root.loginFailed(qsTr("Şifre yanlış")));
    }

    function logout() {
        console.log("Önizleme: oturum kapatılacaktı");
    }

    function reboot() {
        console.log("Önizleme: sistem yeniden başlatılacaktı");
    }

    function poweroff() {
        console.log("Önizleme: sistem kapatılacaktı");
    }

    function setAutologin(enabled) {
        autologin = enabled;
    }

    function changePassword(current, next) {
        later(() => {
            if (current !== root.password)
                root.passwordUpdateFailed(qsTr("Mevcut şifre yanlış"));
            else if (next.length < 4)
                root.passwordUpdateFailed(qsTr("Yeni şifre en az 4 karakter olmalı"));
            else
                root.passwordUpdated();
        });
    }
}
