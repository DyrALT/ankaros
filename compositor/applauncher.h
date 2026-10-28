#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

// Uygulamaları compositor'a bağlanacak şekilde başlatır.
// QML'den AppLauncher.launch() ile kullanılır.
class AppLauncher : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    using QObject::QObject;

    // command: "foot" ya da "foot --title deneme" gibi bir komut satırı.
    // waylandDisplay: compositor'ın soket adı (WaylandCompositor.socketName).
    Q_INVOKABLE bool launch(const QString &command, const QString &waylandDisplay) const;
};
