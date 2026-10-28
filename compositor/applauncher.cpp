#include "applauncher.h"

#include <QLoggingCategory>
#include <QProcess>
#include <QProcessEnvironment>

Q_LOGGING_CATEGORY(lcLauncher, "ankaros.launcher")

bool AppLauncher::launch(const QString &command, const QString &waylandDisplay) const
{
    QStringList arguments = QProcess::splitCommand(command);
    if (arguments.isEmpty()) {
        qCWarning(lcLauncher) << "Boş komut";
        return false;
    }

    QProcess process;
    process.setProgram(arguments.takeFirst());
    process.setArguments(arguments);

    QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
    environment.insert(QStringLiteral("WAYLAND_DISPLAY"), waylandDisplay);
    // Compositor'ın kendisi eglfs ile ekrana çiziyor. Bu ayar uygulamalara geçerse Qt
    // uygulamaları da ekranı ele geçirmeye çalışır, onlar Wayland istemcisi olmalı.
    environment.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("wayland"));
    environment.remove(QStringLiteral("QT_QPA_EGLFS_INTEGRATION"));
    process.setProcessEnvironment(environment);

    qint64 pid = 0;
    if (!process.startDetached(&pid)) {
        qCWarning(lcLauncher) << "Başlatılamadı:" << command << process.errorString();
        return false;
    }
    qCInfo(lcLauncher) << "Başlatıldı:" << command << "pid" << pid;
    return true;
}
