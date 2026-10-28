#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

#include <functional>

// Oturum bilgisi ve oturum yöneticisiyle (ankaros-session) konuşma, bkz. docs/oturum.md.
// Kabuk bu nesneyi Desktop.session olarak kullanır. İstekler /run/ankaros/session.sock'a gider,
// cevaplar sinyallerle gelir. Şifreler yalnızca sokete yazılır, hiçbir yerde saklanmaz.
class Session : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Compositor'ı çalıştıran kullanıcı. Giriş ekranında giriş yapılacak kullanıcı.
    Q_PROPERTY(QString userName READ userName CONSTANT)
    // true: giriş ekranı olarak çalışıyor (ankaros-compositor --login)
    Q_PROPERTY(bool greeter READ greeter CONSTANT)
    // Açılışta otomatik giriş açık mı (session.conf)
    Q_PROPERTY(bool autologin READ autologin NOTIFY autologinChanged)

public:
    explicit Session(QObject *parent = nullptr);

    QString userName() const { return m_userName; }
    bool greeter() const { return m_greeter; }
    bool autologin() const { return m_autologin; }

    Q_INVOKABLE void login(const QString &user, const QString &password);
    Q_INVOKABLE void unlock(const QString &password);
    Q_INVOKABLE void logout();
    Q_INVOKABLE void reboot();
    Q_INVOKABLE void poweroff();
    Q_INVOKABLE void setAutologin(bool enabled);
    Q_INVOKABLE void changePassword(const QString &current, const QString &next);
    // Açılış süresi ölçümü için masaüstünün ilk karesini init'e bildirir
    Q_INVOKABLE void markBoot(const QString &name);

signals:
    void autologinChanged();
    void loginFailed(const QString &message);
    void unlocked();
    void unlockFailed(const QString &message);
    void passwordUpdated();
    void passwordUpdateFailed(const QString &message);

private:
    using Reply = std::function<void(const QStringList &lines, const QString &error)>;
    void request(const char *socket, const QStringList &command, Reply reply = {});
    void refresh();

    QString m_userName;
    bool m_greeter = false;
    bool m_autologin = false;
};
