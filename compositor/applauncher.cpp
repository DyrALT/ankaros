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
    // Compositor'ın ölçeği (Retina ekranlarda 2). Uygulamalar ölçeği Wayland çıktısından alır, bu
    // değişken de geçerse iki kez ölçeklenirler.
    environment.remove(QStringLiteral("QT_SCALE_FACTOR"));
    // Qt uygulamaları compositor'dan "kare çizildi" bildirimini (frame callback) 100 ms içinde
    // alamazsa pencereyi gizli sayıp çizmeyi bırakır. Yazılımsal çizimde (QEMU, softpipe) bir ekran
    // karesi bundan uzun sürebiliyor, ekran bir sonraki girdi olayına kadar eski kalıyordu.
    if (!environment.contains(QStringLiteral("QT_WAYLAND_FRAME_CALLBACK_TIMEOUT")))
        environment.insert(QStringLiteral("QT_WAYLAND_FRAME_CALLBACK_TIMEOUT"),
                           QStringLiteral("1000"));
    process.setProcessEnvironment(environment);

    qint64 pid = 0;
    if (!process.startDetached(&pid)) {
        qCWarning(lcLauncher) << "Başlatılamadı:" << command << process.errorString();
        return false;
    }
    qCInfo(lcLauncher) << "Başlatıldı:" << command << "pid" << pid;
    return true;
}
