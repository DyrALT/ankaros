#pragma once

#include <QByteArray>
#include <QObject>
#include <QString>

class QSocketNotifier;

// Sanal makinenin ekranını açan program (UTM, virt-manager) ile pano paylaşımı (docs/pano.md).
//
// SPICE istemcisi sanal makineyle bir virtio-serial kanalı (com.redhat.spice.0) üzerinden
// konuşur. Kanalın öbür ucunda normalde spice-vdagent çalışır, ama o yalnızca X11 panosunu bilir.
// Bu sınıf aynı protokolün (vd_agent.h) pano kısmını konuşur, yalnızca metin (UTF-8):
//   - Mac'te kopyalanınca istemci CLIPBOARD_GRAB gönderir. Metin hemen istenir (REQUEST), gelince
//     hostTextReceived yayılır, compositor onu kendi panosu yapar.
//   - ankaros'ta kopyalanınca setGuestText çağrılır: istemciye GRAB gönderilir, istemci isterse
//     metin gönderilir.
//   - Ajan bağlıyken SPICE fareyi sanal tabletten değil bu kanaldan gönderir (MOUSE_STATE):
//     mouseStateReceived yayılır, compositor olayı kendisi üretir.
class SpiceAgent : public QObject
{
    Q_OBJECT

public:
    // Kanal yoksa (SPICE'sız sanal makine, gerçek bilgisayar) ya da açılamazsa nullptr döner
    static SpiceAgent *open(QObject *parent);
    ~SpiceAgent() override;

    void setGuestText(const QString &text);

signals:
    void hostTextReceived(const QString &text);
    // x, y: ekranın (display) piksel koordinatı. buttons: vd_agent.h'deki VD_AGENT_*BUTTON_MASK
    void mouseStateReceived(int x, int y, quint32 buttons, int display);

private:
    SpiceAgent(int fd, QObject *parent);

    void readAvailable();
    void handleMessage(quint32 type, const QByteArray &data);
    void sendMessage(quint32 type, const QByteArray &data);
    void flush();
    void announceCapabilities(bool request);
    bool clientHas(int capability) const;

    int m_fd = -1;
    QSocketNotifier *m_reader = nullptr;
    QSocketNotifier *m_writer = nullptr;
    QByteArray m_input;   // kanaldan gelen, henüz parçalara ayrılmamış baytlar
    QByteArray m_message; // parçalardan birleştirilen ileti akışı
    QByteArray m_output;  // gönderilmeyi bekleyen
    QList<quint32> m_clientCapabilities;
    QString m_guestText;
    bool m_guestOwns = false; // pano ankaros'ta kopyalanan metin mi (istemci isterse gönderilir)
};
