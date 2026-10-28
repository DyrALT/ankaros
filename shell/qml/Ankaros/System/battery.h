#pragma once

#include <QObject>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

// Pil durumu (/sys/class/power_supply). Pil yoksa (masaüstü bilgisayar, sanal makine) present false
// olur ve alt çubukta pil simgesi görünmez. Dakikada bir yeniden okunur.
class Battery : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool present READ present NOTIFY changed)
    // Doluluk yüzdesi (0-100)
    Q_PROPERTY(int level READ level NOTIFY changed)
    Q_PROPERTY(bool charging READ charging NOTIFY changed)

public:
    explicit Battery(QObject *parent = nullptr);

    bool present() const { return m_present; }
    int level() const { return m_level; }
    bool charging() const { return m_charging; }

signals:
    void changed();

private:
    void read();

    QTimer m_timer;
    bool m_present = false;
    int m_level = 0;
    bool m_charging = false;
};
