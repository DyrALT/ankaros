#include "appearance.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

#include <utility>

namespace {
const QString group = QStringLiteral("appearance");
}

QString Appearance::filePath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) +
           QStringLiteral("/ankaros/shell.conf");
}

Appearance::Appearance(QObject *parent)
    : QObject(parent)
    , m_settings(filePath(), QSettings::IniFormat)
{
    load();
    // QSettings dosyayı yeni bir dosya yazıp yerine taşıyarak kaydeder: dosyanın kendisi izlenirse
    // ilk kayıttan sonra izleme kopar. Bu yüzden klasör de izlenir, her değişiklikte dosya yeniden
    // eklenir.
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [this] {
        load();
        watch();
    });
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this] {
        load();
        watch();
    });
    watch();
}

void Appearance::watch()
{
    const QFileInfo file(filePath());
    QDir().mkpath(file.absolutePath());
    if (!m_watcher.directories().contains(file.absolutePath()))
        m_watcher.addPath(file.absolutePath());
    if (file.exists() && !m_watcher.files().contains(file.absoluteFilePath()))
        m_watcher.addPath(file.absoluteFilePath());
}

void Appearance::load()
{
    m_settings.sync();
    m_settings.beginGroup(group);
    const auto text = [this](const char *key, const char *fallback) {
        return m_settings.value(QLatin1String(key), QLatin1String(fallback)).toString();
    };
    const auto flag = [this](const char *key, bool fallback) {
        return m_settings.value(QLatin1String(key), fallback).toBool();
    };
    bool darkMode = flag("darkMode", false);
    const QString side = text("windowButtonSide", "right");
    const QString style = text("windowButtonStyle", "icons");
    QString wallpaper = text("wallpaper", "ankara-tasi");
    const bool wallpaperOnLock = flag("wallpaperOnLock", true);
    const QStringList pinnedApps = m_settings.value(QStringLiteral("pinnedApps")).toStringList();
    const bool customPinnedApps = m_settings.contains(QStringLiteral("pinnedApps"));
    const QString clockFormat = text("clockFormat", "24");
    const bool showDate = flag("showDate", true);
    m_settings.endGroup();

    // Giriş ekranı: oturum yöneticisinin ilettiği kullanıcı tercihleri (bkz. appearance.h)
    if (qEnvironmentVariableIsSet("ANKAROS_DARK_MODE"))
        darkMode = qEnvironmentVariable("ANKAROS_DARK_MODE") == QLatin1String("true");
    if (qEnvironmentVariableIsSet("ANKAROS_WALLPAPER"))
        wallpaper = qEnvironmentVariable("ANKAROS_WALLPAPER");

    if (darkMode == m_darkMode && side == m_windowButtonSide && style == m_windowButtonStyle &&
        wallpaper == m_wallpaper && wallpaperOnLock == m_wallpaperOnLock &&
        pinnedApps == m_pinnedApps && customPinnedApps == m_customPinnedApps &&
        clockFormat == m_clockFormat && showDate == m_showDate)
        return;
    m_darkMode = darkMode;
    m_windowButtonSide = side;
    m_windowButtonStyle = style;
    m_wallpaper = wallpaper;
    m_wallpaperOnLock = wallpaperOnLock;
    m_pinnedApps = pinnedApps;
    m_customPinnedApps = customPinnedApps;
    m_clockFormat = clockFormat;
    m_showDate = showDate;
    emit changed();
}

void Appearance::write(const QString &key, const QVariant &value)
{
    m_settings.beginGroup(group);
    m_settings.setValue(key, value);
    m_settings.endGroup();
    m_settings.sync();
}

namespace {

template <typename T> bool assign(T &member, const T &value)
{
    if (member == value)
        return false;
    member = value;
    return true;
}

} // namespace

void Appearance::setDarkMode(bool darkMode)
{
    if (assign(m_darkMode, darkMode)) {
        write(QStringLiteral("darkMode"), darkMode);
        emit changed();
    }
}

void Appearance::setWindowButtonSide(const QString &side)
{
    if (assign(m_windowButtonSide, side)) {
        write(QStringLiteral("windowButtonSide"), side);
        emit changed();
    }
}

void Appearance::setWindowButtonStyle(const QString &style)
{
    if (assign(m_windowButtonStyle, style)) {
        write(QStringLiteral("windowButtonStyle"), style);
        emit changed();
    }
}

void Appearance::setWallpaper(const QString &wallpaper)
{
    if (assign(m_wallpaper, wallpaper)) {
        write(QStringLiteral("wallpaper"), wallpaper);
        emit changed();
    }
}

void Appearance::setWallpaperOnLock(bool enabled)
{
    if (assign(m_wallpaperOnLock, enabled)) {
        write(QStringLiteral("wallpaperOnLock"), enabled);
        emit changed();
    }
}

void Appearance::setPinnedApps(const QStringList &apps)
{
    const bool wasCustom = std::exchange(m_customPinnedApps, true);
    if (assign(m_pinnedApps, apps) || !wasCustom) {
        write(QStringLiteral("pinnedApps"), apps);
        emit changed();
    }
}

void Appearance::setClockFormat(const QString &format)
{
    if (assign(m_clockFormat, format)) {
        write(QStringLiteral("clockFormat"), format);
        emit changed();
    }
}

void Appearance::setShowDate(bool show)
{
    if (assign(m_showDate, show)) {
        write(QStringLiteral("showDate"), show);
        emit changed();
    }
}
