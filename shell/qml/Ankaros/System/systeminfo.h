#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

// Sistem bilgisi (Ayarlar › Hakkında): sürüm, çekirdek, işlemci, bellek, disk, ekran kartı ve
// açılış süresi. Değerler ekranda gösterilecek biçimde, Türkçe sayı biçimiyle hazırlanır ("2 GB",
// "1,3 sn").
class SystemInfo : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(QString version READ version CONSTANT)
    Q_PROPERTY(QString kernel READ kernel CONSTANT)
    Q_PROPERTY(QString processor READ processor CONSTANT)
    Q_PROPERTY(QString memory READ memory CONSTANT)
    Q_PROPERTY(QString disk READ disk NOTIFY changed)
    // OpenGL çizicisinin adı, ör. "virgl (ANGLE Metal Renderer: Apple M2 Pro)"
    Q_PROPERTY(QString graphics READ graphics NOTIFY changed)
    // Çekirdeğin başlamasından masaüstünün ilk karesine (ankarosctl boot)
    Q_PROPERTY(QString bootTime READ bootTime NOTIFY changed)

public:
    explicit SystemInfo(QObject *parent = nullptr);

    QString version() const { return m_version; }
    QString kernel() const { return m_kernel; }
    QString processor() const { return m_processor; }
    QString memory() const { return m_memory; }
    QString disk() const { return m_disk; }
    QString graphics() const { return m_graphics; }
    QString bootTime() const { return m_bootTime; }

    // Metni panoya kopyalar ("Bilgileri kopyala")
    Q_INVOKABLE void copy(const QString &text) const;

signals:
    void changed();

private:
    void readGraphics();

    QString m_version;
    QString m_kernel;
    QString m_processor;
    QString m_memory;
    QString m_disk;
    QString m_graphics;
    QString m_bootTime;
};
