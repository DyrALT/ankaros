#include "audio.h"

#include "wpctl.h"

namespace {
const QString wpctlProgram = QStringLiteral("wpctl");
const QString defaultSink = QStringLiteral("@DEFAULT_AUDIO_SINK@");
constexpr int pollInterval = 2000;
constexpr int maxRetryInterval = 60000;
constexpr int processTimeout = 5000;
} // namespace

Audio::Audio(QObject *parent)
    : QObject(parent)
{
    connect(&m_reader, &QProcess::finished, this,
            [this](int exitCode, QProcess::ExitStatus status) { readFinished(exitCode, status); });
    connect(&m_reader, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error == QProcess::FailedToStart)
            startFailed();
    });
    connect(&m_writer, &QProcess::finished, this, [this](int, QProcess::ExitStatus) {
        m_writeTimeout.stop();
        if (m_pendingVolume >= 0 || m_pendingMuted >= 0)
            applyPending();
        else
            read();
    });
    connect(&m_writer, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        // finished sinyali gelmez; bekleyen başka yazma varsa sırasını kaçırmasın
        if (error != QProcess::FailedToStart)
            return;
        startFailed();
        // QProcess hâlâ Starting durumunda, kendi sinyali içinden start() düşer: olay döngüsüne
        // ertele
        QTimer::singleShot(0, this, [this] {
            if ((m_pendingVolume >= 0 || m_pendingMuted >= 0) &&
                m_writer.state() == QProcess::NotRunning)
                applyPending();
        });
    });
    // Takılan wpctl sonsuza dek beklemesin: öldürülünce finished gelir ve sıra ilerler
    for (QTimer *timeout : {&m_readTimeout, &m_writeTimeout}) {
        timeout->setInterval(processTimeout);
        timeout->setSingleShot(true);
    }
    connect(&m_readTimeout, &QTimer::timeout, &m_reader, &QProcess::kill);
    connect(&m_writeTimeout, &QTimer::timeout, &m_writer, &QProcess::kill);
    m_timer.setInterval(pollInterval);
    connect(&m_timer, &QTimer::timeout, this, &Audio::read);
    m_timer.start();
    read();
}

// wpctl yok ya da başlamadı (Mac önizlemesi, PipeWire henüz hazır değil): kullanılamaz say ama
// oturum boyunca vazgeçme, artan aralıkla yeniden dene
void Audio::startFailed()
{
    m_readTimeout.stop();
    m_writeTimeout.stop();
    m_retryInterval = qMin(m_retryInterval * 2, maxRetryInterval);
    m_timer.setInterval(m_retryInterval);
    if (m_available) {
        m_available = false;
        emit changed();
    }
}

QString Audio::icon() const
{
    return QString::fromLatin1(wpctl::iconName(m_volume, m_muted));
}

void Audio::read()
{
    if (m_reader.state() != QProcess::NotRunning || m_writer.state() != QProcess::NotRunning)
        return;
    m_readWriteSeq = m_writeSeq;
    m_readTimeout.start();
    m_reader.start(wpctlProgram, {QStringLiteral("get-volume"), defaultSink});
}

void Audio::readFinished(int exitCode, QProcess::ExitStatus status)
{
    m_readTimeout.stop();
    // Zaman aşımıyla öldürülen okumanın sonucu yok, durum olduğu gibi kalsın
    if (status != QProcess::NormalExit)
        return;
    const wpctl::Volume volume =
        exitCode == 0 ? wpctl::parseVolume(m_reader.readAllStandardOutput().toStdString())
                      : wpctl::Volume{};
    // Okuma başladıktan sonra bir yazma istendiyse (bitmiş olsa bile) okunan değer eskidir ve
    // arayüzü geri sıçratır; sıra numarası bunu yakalar
    if (m_readWriteSeq != m_writeSeq || m_writer.state() != QProcess::NotRunning ||
        m_pendingVolume >= 0 || m_pendingMuted >= 0)
        return;
    if (volume.valid) {
        // wpctl cevap verdi: yeniden deneme aralığını sıfırla
        m_retryInterval = pollInterval;
        m_timer.setInterval(pollInterval);
    }
    if (volume.valid == m_available &&
        (!volume.valid || (volume.percent == m_volume && volume.muted == m_muted)))
        return;
    m_available = volume.valid;
    if (volume.valid) {
        m_volume = volume.percent;
        m_muted = volume.muted;
    }
    emit changed();
}

void Audio::setVolume(int percent)
{
    percent = qBound(0, percent, 100);
    if (!m_available || (percent == m_volume && m_pendingVolume < 0))
        return;
    m_volume = percent;
    m_pendingVolume = percent;
    ++m_writeSeq;
    emit changed();
    if (m_writer.state() == QProcess::NotRunning)
        applyPending();
}

void Audio::setMuted(bool muted)
{
    if (!m_available || (muted == m_muted && m_pendingMuted < 0))
        return;
    m_muted = muted;
    ++m_writeSeq;
    m_pendingMuted = muted ? 1 : 0;
    emit changed();
    if (m_writer.state() == QProcess::NotRunning)
        applyPending();
}

// Bekleyen ayarları sırayla uygular: önce ses düzeyi, sonra sessiz durumu
void Audio::applyPending()
{
    if (m_pendingVolume >= 0) {
        const QString level = QString::fromStdString(wpctl::formatLevel(m_pendingVolume));
        m_pendingVolume = -1;
        // -l 1.0: düzey kaydırıcıdan gelse bile yazılımsal yükseltmeye çıkılmaz
        m_writeTimeout.start();
        m_writer.start(wpctlProgram, {QStringLiteral("set-volume"), QStringLiteral("-l"),
                                      QStringLiteral("1.0"), defaultSink, level});
    } else if (m_pendingMuted >= 0) {
        const QString state = m_pendingMuted == 1 ? QStringLiteral("1") : QStringLiteral("0");
        m_pendingMuted = -1;
        m_writeTimeout.start();
        m_writer.start(wpctlProgram, {QStringLiteral("set-mute"), defaultSink, state});
    }
}
