#pragma once

#include <QObject>
#include <QString>
#include <QtQml/qqmlregistration.h>

// Oturum bilgisi ve ankaros-session ile konuşma (docs/oturum.md). QML'den Session.userName gibi.
class Session : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Compositor'ı çalıştıran kullanıcı
    Q_PROPERTY(QString userName READ userName CONSTANT)
    // true: giriş ekranı olarak çalışıyor (ankaros-compositor --login)
    Q_PROPERTY(bool greeter READ greeter CONSTANT)

public:
    using QObject::QObject;

    QString userName() const;
    bool greeter() const;
};
