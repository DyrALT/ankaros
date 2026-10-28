#include "socketrequest.h"

#include <QLoggingCategory>
#include <QSocketNotifier>
#include <QTimer>

#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

Q_LOGGING_CATEGORY(lcSocket, "ankaros.socket")

namespace SocketRequest {

namespace {
constexpr int requestTimeout = 10000;
} // namespace

QString parse(const QByteArray &response, QStringList &lines)
{
    lines = QString::fromUtf8(response).split(QLatin1Char('\n'), Qt::SkipEmptyParts);
    const QString last = lines.isEmpty() ? QString() : lines.takeLast();
    if (last == QLatin1String("ok"))
        return {};
    return last.startsWith(QLatin1String("hata ")) ? last.mid(5)
                                                   : QStringLiteral("Cevap alınamadı");
}

void send(QObject *context, const char *socketPath, const QStringList &command, Reply reply)
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
        qCWarning(lcSocket) << "Ulaşılamadı:" << socketPath << strerror(errno);
        if (fd >= 0)
            ::close(fd);
        if (reply)
            reply({}, QStringLiteral("Servise ulaşılamadı"));
        return;
    }

    auto *notifier = new QSocketNotifier(fd, QSocketNotifier::Read, context);
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
    QObject::connect(notifier, &QSocketNotifier::activated, notifier, [fd, buffer, finish] {
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

} // namespace SocketRequest
