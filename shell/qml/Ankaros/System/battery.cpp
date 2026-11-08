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
    int total = 0;
    int count = 0;
    bool charging = false;
    const QDir supplies(QStringLiteral("/sys/class/power_supply"));
    for (const QString &name : supplies.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        const QString base = supplies.filePath(name) + QLatin1Char('/');
        if (readValue(base + QLatin1String("type")) != QLatin1String("Battery"))
            continue;
        // scope=Device: fare, klavye gibi çevre birimlerinin pili, sistemin değil. Dosya yoksa
        // (eski çekirdek) sistem pili sayılır.
        if (readValue(base + QLatin1String("scope")) == QLatin1String("Device"))
            continue;
        present = true;
        // Okunamayan pil ortalamaya sıfır olarak girmesin
        bool ok = false;
        const int capacity = readValue(base + QLatin1String("capacity")).toInt(&ok);
        if (ok) {
            total += qBound(0, capacity, 100);
            ++count;
        }
        charging =
            charging || readValue(base + QLatin1String("status")) == QLatin1String("Charging");
    }
    // Birden çok sistem pili varsa doluluk ortalamadır
    const int level = count > 0 ? qRound(double(total) / count) : 0;
    if (present == m_present && level == m_level && charging == m_charging)
        return;
    m_present = present;
    m_level = level;
    m_charging = charging;
    emit changed();
}
