#include "session.h"

#include "socketrequest.h"

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QTimer>

#include <pwd.h>

Q_LOGGING_CATEGORY(lcSession, "ankaros.session")

namespace {

constexpr const char *sessionSocket = "/run/ankaros/session.sock";
constexpr const char *initSocket = "/run/ankaros/init.sock";

// Kontrol protokolü alanları sekmeyle, komutu satır sonuyla ayırır ve boş alanları atlar: böyle bir
// şifre gönderilirse komut sessizce bozulur ("bilinmeyen komut" gibi anlaşılmaz bir hata döner).
// Bu yüzden istemci tarafında açık bir hatayla reddedilir. Boş dize: şifre uygun.
QString passwordProblem(const QString &password)
{
    if (password.isEmpty())
        return QObject::tr("Şifre boş olamaz");
    if (password.contains(QLatin1Char('\t')) || password.contains(QLatin1Char('\n')) ||
        password.contains(QLatin1Char('\r')) || password.contains(QChar(0)))
        return QObject::tr("Şifre sekme, satır sonu ya da NUL içeremez");
    return {};
}

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
    QString problem = user.isEmpty() ? tr("Kullanıcı adı boş olamaz") : passwordProblem(password);
    if (problem.isEmpty() && (user.contains(QLatin1Char('\t')) || user.contains(QLatin1Char('\n'))))
        problem = tr("Kullanıcı adı geçersiz");
    if (!problem.isEmpty()) {
        QTimer::singleShot(0, this, [this, problem] { emit loginFailed(problem); });
        return;
    }
    // Başarılı girişte oturum yöneticisi bu süreci kapatır, başarısızlıkta hata döner.
    request(sessionSocket, {QStringLiteral("login"), user, password},
            [this](const QStringList &, const QString &error) {
                if (!error.isEmpty())
                    emit loginFailed(error);
            });
}

void Session::lock()
{
    request(sessionSocket, {QStringLiteral("lock")}, [](const QStringList &, const QString &error) {
        if (!error.isEmpty())
            qCWarning(lcSession) << "Kilit bildirilemedi:" << error;
    });
}

void Session::unlock(const QString &password)
{
    if (const QString problem = passwordProblem(password); !problem.isEmpty()) {
        QTimer::singleShot(0, this, [this, problem] { emit unlockFailed(problem); });
        return;
    }
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
    QString problem = passwordProblem(current);
    if (problem.isEmpty())
        problem = passwordProblem(next);
    if (!problem.isEmpty()) {
        QTimer::singleShot(0, this, [this, problem] { emit passwordUpdateFailed(problem); });
        return;
    }
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
