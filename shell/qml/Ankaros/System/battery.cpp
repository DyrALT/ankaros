#include "battery.h"

#include <QDir>
#include <QFile>

namespace {

QString readValue(const QString &path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? QString::fromLatin1(file.readAll()).trimmed()
                                          : QString();
}

} // namespace

Battery::Battery(QObject *parent)
    : QObject(parent)
{
    m_timer.setInterval(60 * 1000);
    connect(&m_timer, &QTimer::timeout, this, &Battery::read);
    m_timer.start();
    read();
}

void Battery::read()
{
    bool present = false;
    int level = 0;
    bool charging = false;
    const QDir supplies(QStringLiteral("/sys/class/power_supply"));
    for (const QString &name : supplies.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        const QString base = supplies.filePath(name) + QLatin1Char('/');
        if (readValue(base + QLatin1String("type")) != QLatin1String("Battery"))
            continue;
        present = true;
        level = qBound(0, readValue(base + QLatin1String("capacity")).toInt(), 100);
        charging = readValue(base + QLatin1String("status")) == QLatin1String("Charging");
        break;
    }
    if (present == m_present && level == m_level && charging == m_charging)
        return;
    m_present = present;
    m_level = level;
    m_charging = charging;
    emit changed();
}
