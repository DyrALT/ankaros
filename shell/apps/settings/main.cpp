// Ayarlar uygulaması (karar 0013): Görünüm, Ağ, Kullanıcı... Kabuktan ayrı bir süreç, Wayland
// istemcisi.
//
//   ankaros-settings                 Görünüm sayfasıyla açılır
//   ankaros-settings --page network  belirli bir sayfayla (alt çubuktaki ağ simgesi böyle açar)
//   ankaros-settings --screenshot d  açılıştan kısa süre sonra pencerenin görüntüsünü kaydedip
//   çıkar (CI)
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlExtensionPlugin>

#ifdef ANKAROS_SHELL_STATIC
// Mac'te modüller statik kütüphanedir, eklentileri elle kaydedilir.
Q_IMPORT_QML_PLUGIN(Ankaros_UIPlugin)
Q_IMPORT_QML_PLUGIN(Ankaros_SystemPlugin)
#endif

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationName(QStringLiteral("ankaros-settings"));
    QGuiApplication::setApplicationDisplayName(QStringLiteral("Ayarlar"));
    // Wayland app_id: compositor pencereyi bununla Ayarlar uygulamasına bağlar (apps.json)
    QGuiApplication::setDesktopFileName(QStringLiteral("org.ankaros.settings"));

    const QStringList arguments = QCoreApplication::arguments();
    const qsizetype pageIndex = arguments.indexOf(QLatin1String("--page"));
    const qsizetype shotIndex = arguments.indexOf(QLatin1String("--screenshot"));

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.setInitialProperties({
        {QStringLiteral("page"),
         pageIndex >= 0 ? arguments.value(pageIndex + 1) : QStringLiteral("appearance")},
        {QStringLiteral("screenshot"), shotIndex >= 0 ? arguments.value(shotIndex + 1) : QString()},
    });
    engine.loadFromModule("Ankaros.Apps.Settings", "Main");

    return QGuiApplication::exec();
}
