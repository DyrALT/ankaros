#include "session.h"

#include "socketrequest.h"

#include <QCoreApplication>
#include <QLoggingCategory>

#include <pwd.h>

Q_LOGGING_CATEGORY(lcSession, "ankaros.session")

namespace {

constexpr const char *sessionSocket = "/run/ankaros/session.sock";
constexpr const char *initSocket = "/run/ankaros/init.sock";

// Giriş ekranında gösterilecek kullanıcı: uid'si 1000 ve üstü olan ilk kullanıcı
QString firstLoginUser()
{
    setpwent();
    QString name;
    while (const passwd *entry = getpwent()) {
        if (entry->pw_uid >= 1000 && entry->pw_uid < 60000) {
            name = QString::fromLocal8Bit(entry->pw_name);
            break;
        }
    }
    endpwent();
    return name;
}

} // namespace

Session::Session(QObject *parent)
    : QObject(parent)
    , m_greeter(QCoreApplication::arguments().contains(QLatin1String("--login")))
{
    m_userName = m_greeter ? firstLoginUser() : qEnvironmentVariable("USER");
    if (!m_greeter)
        refresh();
}

void Session::request(const char *socketPath, const QStringList &command, Reply reply)
{
    SocketRequest::send(this, socketPath, command, std::move(reply));
}

void Session::refresh()
{
    request(sessionSocket, {QStringLiteral("status")},
            [this](const QStringList &lines, const QString &error) {
                if (!error.isEmpty())
                    return;
                for (const QString &line : lines) {
                    const QStringList fields = line.split(QLatin1Char('\t'));
                    if (fields.value(0) == QLatin1String("autologin")) {
                        const bool enabled = fields.value(1) == m_userName && !m_userName.isEmpty();
                        if (enabled != m_autologin) {
                            m_autologin = enabled;
                            emit autologinChanged();
                        }
                    }
                }
            });
}

void Session::login(const QString &user, const QString &password)
{
    // Başarılı girişte oturum yöneticisi bu süreci kapatır, başarısızlıkta hata döner.
    request(sessionSocket, {QStringLiteral("login"), user, password},
            [this](const QStringList &, const QString &error) {
                if (!error.isEmpty())
                    emit loginFailed(error);
            });
}

void Session::unlock(const QString &password)
{
    request(sessionSocket, {QStringLiteral("unlock"), password},
            [this](const QStringList &, const QString &error) {
                if (error.isEmpty())
                    emit unlocked();
                else
                    emit unlockFailed(error);
            });
}

void Session::logout()
{
    request(sessionSocket, {QStringLiteral("logout")});
}

void Session::reboot()
{
    request(sessionSocket, {QStringLiteral("reboot")});
}

void Session::poweroff()
{
    request(sessionSocket, {QStringLiteral("poweroff")});
}

void Session::setAutologin(bool enabled)
{
    request(sessionSocket,
            {QStringLiteral("autologin"), enabled ? QStringLiteral("on") : QStringLiteral("off")},
            [this](const QStringList &, const QString &error) {
                if (!error.isEmpty())
                    qCWarning(lcSession) << "Otomatik giriş değiştirilemedi:" << error;
                refresh();
            });
}

void Session::changePassword(const QString &current, const QString &next)
{
    request(sessionSocket, {QStringLiteral("passwd"), current, next},
            [this](const QStringList &, const QString &error) {
                if (error.isEmpty())
                    emit passwordUpdated();
                else
                    emit passwordUpdateFailed(error);
            });
}

void Session::markBoot(const QString &name)
{
    request(initSocket, {QStringLiteral("mark"), name});
}
