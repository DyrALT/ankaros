#include "appearance.h"

#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>

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
    const bool darkMode = m_settings.value(QStringLiteral("darkMode"), false).toBool();
    const QString side =
        m_settings.value(QStringLiteral("windowButtonSide"), QStringLiteral("right")).toString();
    const QString style =
        m_settings.value(QStringLiteral("windowButtonStyle"), QStringLiteral("icons")).toString();
    m_settings.endGroup();
    if (darkMode == m_darkMode && side == m_windowButtonSide && style == m_windowButtonStyle)
        return;
    m_darkMode = darkMode;
    m_windowButtonSide = side;
    m_windowButtonStyle = style;
    emit changed();
}

void Appearance::write(const QString &key, const QVariant &value)
{
    m_settings.beginGroup(group);
    m_settings.setValue(key, value);
    m_settings.endGroup();
    m_settings.sync();
}

void Appearance::setDarkMode(bool darkMode)
{
    if (darkMode == m_darkMode)
        return;
    m_darkMode = darkMode;
    write(QStringLiteral("darkMode"), darkMode);
    emit changed();
}

void Appearance::setWindowButtonSide(const QString &side)
{
    if (side == m_windowButtonSide)
        return;
    m_windowButtonSide = side;
    write(QStringLiteral("windowButtonSide"), side);
    emit changed();
}

void Appearance::setWindowButtonStyle(const QString &style)
{
    if (style == m_windowButtonStyle)
        return;
    m_windowButtonStyle = style;
    write(QStringLiteral("windowButtonStyle"), style);
    emit changed();
}
