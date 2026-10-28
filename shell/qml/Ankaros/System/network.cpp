#include "network.h"

#include "socketrequest.h"

#include <QLoggingCategory>

#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

Q_LOGGING_CATEGORY(lcNetwork, "ankaros.network")

namespace {
constexpr const char *socketPath = "/run/ankaros/network.sock";
constexpr int reconnectInterval = 2000;
} // namespace

Network::Network(QObject *parent)
    : QObject(parent)
{
    m_reconnect.setInterval(reconnectInterval);
    m_reconnect.setSingleShot(true);
    connect(&m_reconnect, &QTimer::timeout, this, &Network::connectToService);
    connectToService();
}

Network::~Network()
{
    disconnectFromService();
}

void Network::connectToService()
{
    // "watch" komutu cevaptan sonra bağlantıyı açık tutar, servis her değişiklikte yeni durumu
    // ve ardından "ok" satırını gönderir.
    m_fd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    qstrncpy(address.sun_path, socketPath, sizeof address.sun_path);
    static const QByteArray watch = "watch\n";
    if (m_fd < 0 || ::connect(m_fd, reinterpret_cast<sockaddr *>(&address), sizeof address) != 0 ||
        ::send(m_fd, watch.constData(), watch.size(), MSG_NOSIGNAL) != watch.size()) {
        disconnectFromService();
        m_reconnect.start();
        return;
    }
    m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &Network::readUpdates);
}

void Network::disconnectFromService()
{
    delete m_notifier;
    m_notifier = nullptr;
    if (m_fd >= 0)
        ::close(m_fd);
    m_fd = -1;
    m_buffer.clear();
    if (m_available) {
        m_available = false;
        emit changed();
    }
}

void Network::readUpdates()
{
    char chunk[4096];
    for (;;) {
        const ssize_t n = ::read(m_fd, chunk, sizeof chunk);
        if (n > 0) {
            m_buffer.append(chunk, n);
            continue;
        }
        if (n < 0 && (errno == EAGAIN || errno == EINTR))
            break;
        qCInfo(lcNetwork) << "Ağ servisiyle bağlantı koptu, yeniden bağlanılacak";
        disconnectFromService();
        m_reconnect.start();
        return;
    }
    // Her güncelleme "ok" satırıyla biter, son tam güncelleme kullanılır
    const qsizetype end = m_buffer.lastIndexOf("ok\n");
    if (end < 0)
        return;
    const QByteArray block = m_buffer.left(end);
    m_buffer.remove(0, end + 3);
    const qsizetype start = block.lastIndexOf("ok\n");
    QHash<QString, QString> values;
    for (const QString &line : QString::fromUtf8(start < 0 ? block : block.mid(start + 3))
                                   .split(QLatin1Char('\n'), Qt::SkipEmptyParts)) {
        const qsizetype tab = line.indexOf(QLatin1Char('\t'));
        if (tab > 0)
            values.insert(line.left(tab), line.mid(tab + 1));
    }
    m_values = values;
    m_available = true;
    emit changed();
}

void Network::apply(const QString &mode, const QString &address, const QString &gateway,
                    const QString &dns)
{
    QStringList command = {QStringLiteral("set"), mode};
    if (mode == QLatin1String("static"))
        command << address << gateway << dns.simplified();
    SocketRequest::send(this, socketPath, command,
                        [this](const QStringList &, const QString &error) {
                            if (error.isEmpty())
                                emit applied();
                            else
                                emit applyFailed(error);
                        });
}
