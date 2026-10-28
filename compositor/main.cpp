// ankaros-compositor: Wayland sunucusunu ve masaüstünü başlatır.
// Ekrana doğrudan çizer (eglfs/KMS), ortam değişkenleri /etc/profile.d/ankaros-grafik.sh'de.
#include <QGuiApplication>
#include <QQmlApplicationEngine>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("ankaros-compositor"));

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("Ankaros.Compositor", "Main");

    return QGuiApplication::exec();
}
