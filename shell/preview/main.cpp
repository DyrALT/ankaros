// Kabuk önizleme uygulaması. Ankaros.Shell modülünü 1440x900'lük bir pencerede gösterir.
// Pencerenin kısayolları ve yeniden yükleme protokolü için bkz. Main.qml ve
// scripts/shell-preview.sh.
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlExtensionPlugin>

#ifdef ANKAROS_SHELL_STATIC
// Ankaros.Shell statik bir kütüphane olarak derlendiyse eklentisinin elle kaydedilmesi gerekir.
// Paylaşılan modül olarak kurulduysa (ankaros imajı) Qt onu QML klasöründen kendisi bulur.
Q_IMPORT_QML_PLUGIN(Ankaros_ShellPlugin)
#endif

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("ankaros-shell-preview"));

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.loadFromModule("Ankaros.Shell.Preview", "Main");

    return QGuiApplication::exec();
}
