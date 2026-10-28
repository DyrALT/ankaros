#include "systeminfo.h"

#include "socketrequest.h"

#include <QClipboard>
#include <QFile>
#include <QGuiApplication>
#include <QLocale>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QRegularExpression>
#include <QThread>
#include <QTimer>

#include <sys/statvfs.h>
#include <sys/utsname.h>

namespace {

const QLocale turkish(QLocale::Turkish, QLocale::Turkey);

QString readFile(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? QString::fromUtf8(file.readAll()) : QString();
}

// 1536 MB -> "1,5 GB", 768 MB -> "768 MB"
QString size(double bytes)
{
    const double megabytes = bytes / (1024.0 * 1024.0);
    if (megabytes < 1024)
        return turkish.toString(megabytes, 'f', 0) + QStringLiteral(" MB");
    const double gigabytes = megabytes / 1024.0;
    return turkish.toString(gigabytes, 'f',
                            gigabytes < 10 && gigabytes != qRound(gigabytes) ? 1 : 0) +
           QStringLiteral(" GB");
}

} // namespace

SystemInfo::SystemInfo(QObject *parent)
    : QObject(parent)
{
    // /etc/os-release: PRETTY_NAME="ankaros 0.3 (Faz 2)" -> "0.3 · Faz 2"
    const QString osRelease = readFile(QStringLiteral("/etc/os-release"));
    const auto pretty =
        QRegularExpression(QStringLiteral("PRETTY_NAME=\"?ankaros ([^\"(]+)\\(?([^\")]*)"))
            .match(osRelease);
    m_version = pretty.hasMatch() ? pretty.captured(1).trimmed() +
                                        (pretty.captured(2).isEmpty()
                                             ? QString()
                                             : QStringLiteral(" · ") + pretty.captured(2))
                                  : QStringLiteral("-");

    utsname name{};
    if (uname(&name) == 0)
        m_kernel = QStringLiteral("Linux %1 (%2)")
                       .arg(QString::fromLatin1(name.release), QString::fromLatin1(name.machine));
    m_processor = tr("%1 çekirdek").arg(QThread::idealThreadCount());

    const auto total = QRegularExpression(QStringLiteral("MemTotal:\\s+(\\d+) kB"))
                           .match(readFile(QStringLiteral("/proc/meminfo")));
    m_memory = total.hasMatch() ? size(total.captured(1).toDouble() * 1024) : QStringLiteral("-");

    struct statvfs root{};
    if (statvfs("/", &root) == 0 && root.f_blocks > 0) {
        const double capacity = double(root.f_blocks) * root.f_frsize;
        const double used = double(root.f_blocks - root.f_bfree) * root.f_frsize;
        m_disk = tr("%1, %%2 dolu").arg(size(capacity)).arg(qRound(100 * used / capacity));
    }

    // Ekran kartı ve açılış süresi pencere açıldıktan sonra okunur, açılışı geciktirmesin
    QTimer::singleShot(0, this, &SystemInfo::readGraphics);
    SocketRequest::send(this, "/run/ankaros/init.sock", {QStringLiteral("boot")},
                        [this](const QStringList &lines, const QString &error) {
                            if (!error.isEmpty())
                                return;
                            for (const QString &line : lines) {
                                const QStringList fields = line.split(QLatin1Char('\t'));
                                if (fields.value(0) == QLatin1String("mark") &&
                                    fields.value(1) == QStringLiteral("masaüstü")) {
                                    m_bootTime =
                                        turkish.toString(fields.value(2).toDouble(), 'f', 1) +
                                        QStringLiteral(" sn");
                                    emit changed();
                                }
                            }
                        });
}

void SystemInfo::readGraphics()
{
    QOffscreenSurface surface;
    surface.create();
    QOpenGLContext context;
    if (!context.create() || !context.makeCurrent(&surface)) {
        m_graphics = QStringLiteral("-");
        emit changed();
        return;
    }
    QString renderer = QString::fromLatin1(
        reinterpret_cast<const char *>(context.functions()->glGetString(GL_RENDERER)));
    context.doneCurrent();
    // "virgl (ANGLE (Apple, ANGLE Metal Renderer: Apple M2 Pro, Version ...))" -> kısalt
    renderer.remove(QRegularExpression(QStringLiteral(",\\s*(Version|Unspecified).*")));
    renderer.replace(QStringLiteral("ANGLE (Apple, ANGLE Metal Renderer: "), QStringLiteral(""));
    m_graphics = renderer.trimmed();
    if (m_graphics.count(QLatin1Char('(')) > m_graphics.count(QLatin1Char(')')))
        m_graphics += QLatin1Char(')');
    emit changed();
}

void SystemInfo::copy(const QString &text) const
{
    QGuiApplication::clipboard()->setText(text);
}
