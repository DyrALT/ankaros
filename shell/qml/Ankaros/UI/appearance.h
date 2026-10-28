#pragma once

#include <QFileSystemWatcher>
#include <QObject>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QtQml/qqmlregistration.h>

// Kullanıcının görünüm ayarları (karar 0015): tema, pencere düğmeleri, duvar kağıdı, alt çubuk.
// ~/.config/ankaros/shell.conf dosyasının [appearance] bölümünde saklanır. Dosya izlenir: bir süreç
// (ör. Ayarlar uygulaması) değiştirince diğerleri (masaüstü, açık uygulamalar) hemen güncellenir.
//
// Giriş ekranı başka bir kullanıcıyla çalıştığı için kullanıcının dosyasını okuyamaz. Oturum
// yöneticisi tema ve duvar kağıdını ona ANKAROS_DARK_MODE ve ANKAROS_WALLPAPER ortam
// değişkenleriyle iletir.
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
    // Duvar kağıdı: "ankara-tasi", "bozkir", "kehribar", "gece" (Wallpaper.qml)
    Q_PROPERTY(QString wallpaper READ wallpaper WRITE setWallpaper NOTIFY changed)
    // Duvar kağıdı kilit ve giriş ekranında da kullanılsın mı (değilse varsayılan Ankara Taşı)
    Q_PROPERTY(bool wallpaperOnLock READ wallpaperOnLock WRITE setWallpaperOnLock NOTIFY changed)
    // Alt çubuğa sabitlenmiş uygulamalar, sırasıyla. Kullanıcı hiç seçmediyse (customPinnedApps
    // false) apps.json'daki varsayılan kullanılır, hepsini kaldırdıysa liste boştur.
    Q_PROPERTY(QStringList pinnedApps READ pinnedApps WRITE setPinnedApps NOTIFY changed)
    Q_PROPERTY(bool customPinnedApps READ customPinnedApps NOTIFY changed)
    // Saat biçimi: "24" ya da "12"
    Q_PROPERTY(QString clockFormat READ clockFormat WRITE setClockFormat NOTIFY changed)
    Q_PROPERTY(bool showDate READ showDate WRITE setShowDate NOTIFY changed)
    // Son kullanılan uygulamalar, en yeniden eskiye: [{ app: "Terminal", time: Date }, ...]
    // Başlat panelinin "Son kullanılanlar" bölümü. noteAppUsed() ile eklenir.
    Q_PROPERTY(QVariantList recentApps READ recentApps NOTIFY changed)
    // Ayarlar uygulamasından açık penceresinde bir sayfaya geçmesi istendiğinde ("sayfa|zaman").
    // Başlat panelinde bir ayar arandığında Ayarlar zaten açıksa compositor bunu yazar, Ayarlar
    // dosyayı izlediği için hemen o sayfaya geçer. Zaman, aynı sayfa ikinci kez istendiğinde de
    // değişiklik olsun diyedir.
    Q_PROPERTY(QString settingsRequest READ settingsRequest NOTIFY changed)

public:
    explicit Appearance(QObject *parent = nullptr);

    bool darkMode() const { return m_darkMode; }
    QString windowButtonSide() const { return m_windowButtonSide; }
    QString windowButtonStyle() const { return m_windowButtonStyle; }
    QString wallpaper() const { return m_wallpaper; }
    bool wallpaperOnLock() const { return m_wallpaperOnLock; }
    QStringList pinnedApps() const { return m_pinnedApps; }
    bool customPinnedApps() const { return m_customPinnedApps; }
    QString clockFormat() const { return m_clockFormat; }
    bool showDate() const { return m_showDate; }
    QVariantList recentApps() const;
    QString settingsRequest() const { return m_settingsRequest; }

    void setDarkMode(bool darkMode);
    void setWindowButtonSide(const QString &side);
    void setWindowButtonStyle(const QString &style);
    void setWallpaper(const QString &wallpaper);
    void setWallpaperOnLock(bool enabled);
    void setPinnedApps(const QStringList &apps);
    void setClockFormat(const QString &format);
    void setShowDate(bool show);
    Q_INVOKABLE void noteAppUsed(const QString &app);
    Q_INVOKABLE void requestSettingsPage(const QString &page);

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
    QString m_wallpaper = QStringLiteral("ankara-tasi");
    bool m_wallpaperOnLock = true;
    // "zaman (ms)|uygulama" biçiminde, en yeniden eskiye
    QStringList m_recentApps;
    QString m_settingsRequest;
    QStringList m_pinnedApps;
    bool m_customPinnedApps = false;
    QString m_clockFormat = QStringLiteral("24");
    bool m_showDate = true;
};
