#include "spiceagent.h"

#include <QSocketNotifier>
#include <QTimer>
#include <QtEndian>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

namespace {

// spice-protocol, spice/vd_agent.h
constexpr quint32 clientPort = 1; // VDP_CLIENT_PORT
constexpr quint32 agentProtocol = 1;
constexpr int chunkHeaderSize = 8;    // VDIChunkHeader: port, size
constexpr int messageHeaderSize = 20; // VDAgentMessage: protocol, type, opaque (64 bit), size
constexpr int maxChunkData = 2048;    // VD_AGENT_MAX_DATA_SIZE

enum MessageType : quint32
{
    Clipboard = 4,
    AnnounceCapabilities = 6,
    ClipboardGrab = 7,
    ClipboardRequest = 8,
    ClipboardRelease = 9,
};

enum Capability
{
    CapClipboardByDemand = 5,
    CapGuestLineEndLf = 8,
};

enum ClipboardType : quint32
{
    TypeNone = 0,
    TypeUtf8Text = 1,
};

// Daha büyük pano içeriği ve bozuk boyut alanları kabul edilmez (bellek tükenmesin)
constexpr int maxMessage = 8 * 1024 * 1024;
constexpr int maxPending = 16 * 1024 * 1024;

const char *portPath = "/dev/virtio-ports/com.redhat.spice.0";

void appendU32(QByteArray &bytes, quint32 value)
{
    const quint32 little = qToLittleEndian(value);
    bytes.append(reinterpret_cast<const char *>(&little), sizeof little);
}

quint32 readU32(const QByteArray &bytes, int offset)
{
    return qFromLittleEndian<quint32>(bytes.constData() + offset);
}

} // namespace

SpiceAgent *SpiceAgent::open(QObject *parent)
{
    const int fd = ::open(portPath, O_RDWR | O_NONBLOCK | O_CLOEXEC | O_NOCTTY);
    if (fd < 0) {
        if (errno != ENOENT)
            qWarning("pano: %s açılamadı: %s", portPath, std::strerror(errno));
        return nullptr;
    }
    return new SpiceAgent(fd, parent);
}

SpiceAgent::SpiceAgent(int fd, QObject *parent)
    : QObject(parent)
    , m_fd(fd)
{
    m_reader = new QSocketNotifier(fd, QSocketNotifier::Read, this);
    connect(m_reader, &QSocketNotifier::activated, this, &SpiceAgent::readAvailable);
    m_writer = new QSocketNotifier(fd, QSocketNotifier::Write, this);
    m_writer->setEnabled(false);
    connect(m_writer, &QSocketNotifier::activated, this, &SpiceAgent::flush);
    // İstemci bağlıysa yetenekleri hemen öğrenir. Sonradan bağlanırsa kendisi sorar.
    announceCapabilities(true);
}

SpiceAgent::~SpiceAgent()
{
    if (m_fd >= 0)
        ::close(m_fd);
}

void SpiceAgent::readAvailable()
{
    char buffer[16384];
    for (;;) {
        const ssize_t length = ::read(m_fd, buffer, sizeof buffer);
        if (length > 0) {
            m_input.append(buffer, length);
            continue;
        }
        if (length < 0 && errno == EINTR)
            continue;
        if (length < 0 && errno == EAGAIN)
            break;
        // İstemci bağlı değil (0 ya da hata): kanal sürekli "okunabilir" görünür, bir süre sonra
        // yeniden bakılır. Yarım kalan ileti atılır, istemci bağlanınca yeteneklerini yeniden
        // duyurur.
        m_input.clear();
        m_message.clear();
        m_clientCapabilities.clear();
        m_reader->setEnabled(false);
        QTimer::singleShot(1000, this, [this] { m_reader->setEnabled(true); });
        break;
    }

    // Parçalar (VDIChunk) birleştirilir, akıştan iletiler çıkarılır
    while (m_input.size() >= chunkHeaderSize) {
        const quint32 size = readU32(m_input, 4);
        if (size > maxChunkData * 64) {
            qWarning("pano: bozuk parça (%u bayt), kanal sıfırlandı", size);
            m_input.clear();
            m_message.clear();
            break;
        }
        if (m_input.size() < chunkHeaderSize + int(size))
            break;
        m_message.append(m_input.constData() + chunkHeaderSize, size);
        m_input.remove(0, chunkHeaderSize + int(size));
    }
    while (m_message.size() >= messageHeaderSize) {
        const quint32 protocol = readU32(m_message, 0);
        const quint32 type = readU32(m_message, 4);
        const quint32 size = readU32(m_message, 16);
        if (protocol != agentProtocol || size > maxMessage) {
            qWarning("pano: tanınmayan ileti (sürüm %u, %u bayt), kanal sıfırlandı", protocol,
                     size);
            m_message.clear();
            break;
        }
        if (m_message.size() < messageHeaderSize + int(size))
            break;
        const QByteArray data = m_message.mid(messageHeaderSize, int(size));
        m_message.remove(0, messageHeaderSize + int(size));
        handleMessage(type, data);
    }
}

void SpiceAgent::handleMessage(quint32 type, const QByteArray &data)
{
    switch (type) {
    case AnnounceCapabilities: {
        if (data.size() < 4)
            return;
        m_clientCapabilities.clear();
        for (int offset = 4; offset + 4 <= data.size(); offset += 4)
            m_clientCapabilities.append(readU32(data, offset));
        if (readU32(data, 0) != 0)
            announceCapabilities(false);
        // ankaros'ta kopyalanmış bir metin varsa yeni bağlanan istemci de bilsin
        if (m_guestOwns)
            setGuestText(m_guestText);
        return;
    }
    case ClipboardGrab:
        // Mac'te kopyalandı: metin varsa hemen istenir (ankaros'ta yapıştırma anında beklemek
        // compositor'ın panosunu bloklardı)
        for (int offset = 0; offset + 4 <= data.size(); offset += 4) {
            if (readU32(data, offset) == TypeUtf8Text) {
                m_guestOwns = false;
                QByteArray request;
                appendU32(request, TypeUtf8Text);
                sendMessage(ClipboardRequest, request);
                return;
            }
        }
        return;
    case Clipboard: {
        if (data.size() < 4 || readU32(data, 0) != TypeUtf8Text)
            return;
        QByteArray text = data.mid(4);
        // Bazı istemciler metnin sonuna NUL koyar
        while (text.endsWith('\0'))
            text.chop(1);
        emit hostTextReceived(QString::fromUtf8(text));
        return;
    }
    case ClipboardRequest: {
        QByteArray reply;
        if (data.size() >= 4 && readU32(data, 0) == TypeUtf8Text && m_guestOwns) {
            appendU32(reply, TypeUtf8Text);
            reply.append(m_guestText.toUtf8());
        } else {
            appendU32(reply, TypeNone);
        }
        sendMessage(Clipboard, reply);
        return;
    }
    case ClipboardRelease:
    default:
        return;
    }
}

void SpiceAgent::setGuestText(const QString &text)
{
    if (text.isEmpty())
        return;
    m_guestText = text;
    m_guestOwns = true;
    if (!clientHas(CapClipboardByDemand))
        return;
    QByteArray grab;
    appendU32(grab, TypeUtf8Text);
    sendMessage(ClipboardGrab, grab);
}

bool SpiceAgent::clientHas(int capability) const
{
    const int word = capability / 32;
    return word < m_clientCapabilities.size() &&
           (m_clientCapabilities.at(word) & (1u << (capability % 32))) != 0;
}

void SpiceAgent::announceCapabilities(bool request)
{
    QByteArray data;
    appendU32(data, request ? 1 : 0);
    appendU32(data, (1u << CapClipboardByDemand) | (1u << CapGuestLineEndLf));
    sendMessage(AnnounceCapabilities, data);
}

void SpiceAgent::sendMessage(quint32 type, const QByteArray &data)
{
    QByteArray message;
    appendU32(message, agentProtocol);
    appendU32(message, type);
    appendU32(message, 0); // opaque
    appendU32(message, 0);
    appendU32(message, quint32(data.size()));
    message.append(data);

    if (m_output.size() + message.size() > maxPending) {
        // İstemci uzun süredir okumuyor: eski iletiler anlamsız
        m_output.clear();
    }
    for (int offset = 0; offset < message.size(); offset += maxChunkData) {
        const int length = qMin(maxChunkData, int(message.size()) - offset);
        appendU32(m_output, clientPort);
        appendU32(m_output, quint32(length));
        m_output.append(message.constData() + offset, length);
    }
    flush();
}

void SpiceAgent::flush()
{
    while (!m_output.isEmpty()) {
        const ssize_t written = ::write(m_fd, m_output.constData(), size_t(m_output.size()));
        if (written > 0) {
            m_output.remove(0, int(written));
            continue;
        }
        if (written < 0 && errno == EINTR)
            continue;
        if (written < 0 && errno != EAGAIN) {
            // İstemci bağlı değil: bekleyenler atılır, bağlanınca yetenekler yeniden duyurulur
            m_output.clear();
            break;
        }
        break;
    }
    m_writer->setEnabled(!m_output.isEmpty());
}
