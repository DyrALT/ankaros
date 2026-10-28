#include "session.h"

#include <QCoreApplication>
#include <QLoggingCategory>
#include <QSocketNotifier>
#include <QTimer>

#include <cerrno>
#include <cstring>
#include <pwd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

Q_LOGGING_CATEGORY(lcSession, "ankaros.session")

namespace {

constexpr const char *sessionSocket = "/run/ankaros/session.sock";
constexpr const char *initSocket = "/run/ankaros/init.sock";
constexpr int requestTimeout = 10000;

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
    // Unix soketi: QtNetwork gerekmesin diye doğrudan POSIX, okuma QSocketNotifier ile olay
    // döngüsünde.
    const int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    qstrncpy(address.sun_path, socketPath, sizeof address.sun_path);
    const QByteArray line = command.join(QLatin1Char('\t')).toUtf8() + '\n';
    if (fd < 0 || ::connect(fd, reinterpret_cast<sockaddr *>(&address), sizeof address) != 0 ||
        ::send(fd, line.constData(), line.size(), MSG_NOSIGNAL) != line.size()) {
        qCWarning(lcSession) << "Ulaşılamadı:" << socketPath << strerror(errno);
        if (fd >= 0)
            ::close(fd);
        if (reply)
            reply({}, QStringLiteral("Oturum yöneticisine ulaşılamadı"));
        return;
    }

    auto *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    auto *buffer = new QByteArray;
    auto finish = [notifier, buffer, fd, reply](const QString &failure) {
        notifier->setEnabled(false);
        ::close(fd);
        if (reply) {
            QStringList lines =
                QString::fromUtf8(*buffer).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
            QString error = failure;
            const QString last = lines.isEmpty() ? QString() : lines.takeLast();
            if (error.isEmpty() && last != QLatin1String("ok"))
                error = last.startsWith(QLatin1String("hata ")) ? last.mid(5)
                                                                : QStringLiteral("Cevap alınamadı");
            reply(lines, error);
        }
        delete buffer;
        notifier->deleteLater();
    };
    connect(notifier, &QSocketNotifier::activated, notifier, [fd, buffer, finish] {
        char chunk[4096];
        for (;;) {
            const ssize_t n = ::read(fd, chunk, sizeof chunk);
            if (n > 0) {
                buffer->append(chunk, n);
                continue;
            }
            if (n < 0 && (errno == EAGAIN || errno == EINTR))
                return;
            finish({}); // bağlantı kapandı: cevap tamam
            return;
        }
    });
    QTimer::singleShot(requestTimeout, notifier, [notifier, finish] {
        if (notifier->isEnabled())
            finish(QStringLiteral("Cevap gelmedi"));
    });
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
