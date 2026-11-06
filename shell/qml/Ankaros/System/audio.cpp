#include "audio.h"

#include "wpctl.h"

namespace {
const QString wpctlProgram = QStringLiteral("wpctl");
const QString defaultSink = QStringLiteral("@DEFAULT_AUDIO_SINK@");
} // namespace

Audio::Audio(QObject *parent)
    : QObject(parent)
{
    connect(&m_reader, &QProcess::finished, this,
            [this](int exitCode, QProcess::ExitStatus) { readFinished(exitCode); });
    connect(&m_reader, &QProcess::errorOccurred, this, [this](QProcess::ProcessError error) {
        if (error != QProcess::FailedToStart)
            return;
        // wpctl yok (Mac önizlemesi, ses paketi olmayan imaj): sessizce kullanılamaz say
        m_missing = true;
        m_timer.stop();
        if (m_available) {
            m_available = false;
            emit changed();
        }
    });
    connect(&m_writer, &QProcess::finished, this, [this](int, QProcess::ExitStatus) {
        if (m_pendingVolume >= 0 || m_pendingMuted >= 0)
            applyPending();
        else
            read();
    });
    m_timer.setInterval(2000);
    connect(&m_timer, &QTimer::timeout, this, &Audio::read);
    m_timer.start();
    read();
}

QString Audio::icon() const
{
    return QString::fromLatin1(wpctl::iconName(m_volume, m_muted));
}

void Audio::read()
{
    if (m_missing || m_reader.state() != QProcess::NotRunning ||
        m_writer.state() != QProcess::NotRunning)
        return;
    m_reader.start(wpctlProgram, {QStringLiteral("get-volume"), defaultSink});
}

void Audio::readFinished(int exitCode)
{
    const wpctl::Volume volume =
        exitCode == 0 ? wpctl::parseVolume(m_reader.readAllStandardOutput().toStdString())
                      : wpctl::Volume{};
    // Yazma sürerken ya da bekleyen değer varken okunan eski değer arayüzü geri sıçratmasın
    if (m_writer.state() != QProcess::NotRunning || m_pendingVolume >= 0 || m_pendingMuted >= 0)
        return;
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
    emit changed();
    if (m_writer.state() == QProcess::NotRunning)
        applyPending();
}

void Audio::setMuted(bool muted)
{
    if (!m_available || (muted == m_muted && m_pendingMuted < 0))
        return;
    m_muted = muted;
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
        m_writer.start(wpctlProgram, {QStringLiteral("set-volume"), QStringLiteral("-l"),
                                      QStringLiteral("1.0"), defaultSink, level});
    } else if (m_pendingMuted >= 0) {
        const QString state = m_pendingMuted == 1 ? QStringLiteral("1") : QStringLiteral("0");
        m_pendingMuted = -1;
        m_writer.start(wpctlProgram, {QStringLiteral("set-mute"), defaultSink, state});
    }
}
