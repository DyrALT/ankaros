// Terminal (karar 0014): libvterm ile terminal öykünmesi, sekmeler, kopyala-yapıştır. Kabuktan ayrı
// bir süreç, Wayland istemcisi.
//
//   ankaros-terminal                 kullanıcının kabuğuyla bir sekme açar
//   ankaros-terminal --screenshot d  açılıştan kısa süre sonra pencerenin görüntüsünü kaydedip
//   çıkar (CI)
#include <QFontDatabase>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QtQml/QQmlExtensionPlugin>

#ifdef ANKAROS_SHELL_STATIC
// Mac'te modüller statik kütüphanedir, eklentileri elle kaydedilir.
Q_IMPORT_QML_PLUGIN(Ankaros_UIPlugin)
#endif

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QGuiApplication::setOrganizationName(QStringLiteral("ankaros"));
    QGuiApplication::setApplicationName(QStringLiteral("ankaros-terminal"));
    QGuiApplication::setApplicationDisplayName(QStringLiteral("Terminal"));
    // Wayland app_id: compositor pencereyi bununla Terminal uygulamasına bağlar (apps.json)
    QGuiApplication::setDesktopFileName(QStringLiteral("org.ankaros.terminal"));

    // JetBrains Mono, uygulamanın içine gömülü (fonts/, SIL Open Font License)
    for (const char *file : {"Regular", "Bold"}) {
        const QString path =
            QStringLiteral(":/qt/qml/Ankaros/Apps/Terminal/fonts/JetBrainsMono-%1.ttf")
                .arg(QLatin1String(file));
        if (QFontDatabase::addApplicationFont(path) < 0)
            qWarning("ankaros-terminal: yazı tipi yüklenemedi: %s", qPrintable(path));
    }

    const QStringList arguments = QCoreApplication::arguments();
    const qsizetype shotIndex = arguments.indexOf(QLatin1String("--screenshot"));

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine, &QQmlApplicationEngine::objectCreationFailed, &app,
        [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
    engine.setInitialProperties({
        {QStringLiteral("screenshot"), shotIndex >= 0 ? arguments.value(shotIndex + 1) : QString()},
    });
    engine.loadFromModule("Ankaros.Apps.Terminal", "Main");

    return QGuiApplication::exec();
}
