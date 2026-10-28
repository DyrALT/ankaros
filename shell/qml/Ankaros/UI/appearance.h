#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QtQml/qqmlregistration.h>

// Kullanıcının görünüm ayarları (karar 0008, 0013): tema, pencere düğmelerinin konumu ve stili.
// ~/.config/ankaros/shell.conf dosyasının [appearance] bölümünde saklanır. Dosya izlenir: bir süreç
// (ör. Ayarlar uygulaması) değiştirince diğerleri (masaüstü, açık uygulamalar) hemen güncellenir.
class Appearance : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    Q_PROPERTY(bool darkMode READ darkMode WRITE setDarkMode NOTIFY changed)
    // "right" ya da "left" (karar 0007)
    Q_PROPERTY(
        QString windowButtonSide READ windowButtonSide WRITE setWindowButtonSide NOTIFY changed)
    // "icons" (çizgi simgeler) ya da "colored" (renkli daireler)
    Q_PROPERTY(
        QString windowButtonStyle READ windowButtonStyle WRITE setWindowButtonStyle NOTIFY changed)

public:
    explicit Appearance(QObject *parent = nullptr);

    bool darkMode() const { return m_darkMode; }
    QString windowButtonSide() const { return m_windowButtonSide; }
    QString windowButtonStyle() const { return m_windowButtonStyle; }

    void setDarkMode(bool darkMode);
    void setWindowButtonSide(const QString &side);
    void setWindowButtonStyle(const QString &style);

    // Ayar dosyasının yolu (testler ve belgeler için)
    static QString filePath();

signals:
    void changed();

private:
    void load();
    void write(const QString &key, const QVariant &value);
    void watch();

    QSettings m_settings;
    QFileSystemWatcher m_watcher;
    bool m_darkMode = false;
    QString m_windowButtonSide = QStringLiteral("right");
    QString m_windowButtonStyle = QStringLiteral("icons");
};
