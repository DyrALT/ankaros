// ankaros-compositor: Wayland sunucusunu ve masaüstünü başlatır.
// Ekrana doğrudan çizer (eglfs/KMS), ortam değişkenleri /etc/profile.d/ankaros-grafik.sh'de.
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    // Uygulamaların GPU arabellekleri linux-dmabuf ile alınır. Mesa 26'da eglBindWaylandDisplayWL
    // (wl_drm) kalktı, Qt onu yine de ister. Uygulamalara geçmez: applauncher.cpp.
    qputenv("QT_WAYLAND_CLIENT_BUFFER_INTEGRATION", "linux-dmabuf-unstable-v1");
    qputenv("QT_WAYLAND_IGNORE_BIND_DISPLAY", "1");

    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("ankaros-compositor"));

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("Ankaros.Compositor", "Main");

    return QGuiApplication::exec();
}
