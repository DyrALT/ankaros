#pragma once

#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

// Varsayılan ses çıkışının düzeyi ve sessiz durumu (docs/ses.md). WirePlumber'ın wpctl aracıyla
// konuşur; ses altyapısı (PipeWire) ya da ses aygıtı yoksa available false olur ve arayüzde ses
// simgesi görünmez. İki saniyede bir yeniden okunur, böylece başka bir yerde (ör. bir uygulamada)
// yapılan değişiklik de yansır.
class Audio : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool available READ available NOTIFY changed)
    // Ses düzeyi yüzdesi (0-100; 100 üstü yazılımsal yükseltme, arayüz kullanmaz)
    Q_PROPERTY(int volume READ volume WRITE setVolume NOTIFY changed)
    Q_PROPERTY(bool muted READ muted WRITE setMuted NOTIFY changed)
    // Duruma uygun simge adı: volumeMute, volume1 ... volume3 (wpctl.h, iconName)
    Q_PROPERTY(QString icon READ icon NOTIFY changed)

public:
    explicit Audio(QObject *parent = nullptr);

    bool available() const { return m_available; }
    int volume() const { return m_volume; }
    bool muted() const { return m_muted; }
    QString icon() const;

    void setVolume(int percent);
    void setMuted(bool muted);
    Q_INVOKABLE void toggleMuted() { setMuted(!m_muted); }

signals:
    void changed();

private:
    void read();
    void readFinished(int exitCode);
    void applyPending();

    QTimer m_timer;
    QProcess m_reader;
    QProcess m_writer;
    bool m_available = false;
    int m_volume = 0;
    bool m_muted = false;
    // Kaydırıcı sürüklenirken wpctl'nin arkasında kalmamak için son istenen değer tutulur
    int m_pendingVolume = -1;
    int m_pendingMuted = -1;
    bool m_missing = false; // wpctl çalıştırılamadı: bir daha denenmez
};
