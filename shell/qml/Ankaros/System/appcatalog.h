#pragma once

#include <QFileSystemWatcher>
#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QtQml/qqmlregistration.h>

// Başlat panelindeki ve alt çubuktaki uygulamaların listesi. /etc/ankaros/apps.json'dan okunur
// (ANKAROS_APPS ortam değişkeniyle başka bir dosya verilebilir). ank kur ile kurulanlar
// /var/lib/ankaros/uygulamalar.json'dadır (ANKAROS_USER_APPS), dosya değişince liste güncellenir.
// Biçim: docs/uygulama-listesi.md
class AppCatalog : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Bütün uygulamaların adları, dosyadaki sırayla
    Q_PROPERTY(QStringList names READ names NOTIFY changed)
    // Alt çubuğa sabitlenmiş uygulamalar
    Q_PROPERTY(QStringList pinned READ pinned NOTIFY changed)
    // Komutu olan, yani gerçekten açılabilen uygulamalar (dosyadaki sırayla). Başlat panelinde ve
    // alt çubukta yalnızca bunlar gösterilir, diğerleri ileride yazılacak uygulamaların yer
    // tutucusudur.
    Q_PROPERTY(QStringList launchable READ launchable NOTIFY changed)

public:
    explicit AppCatalog(QObject *parent = nullptr);

    QStringList names() const { return m_names; }
    QStringList pinned() const { return m_pinned; }
    QStringList launchable() const;

    // Uygulamanın komutu. Komutu olmayan (henüz yazılmamış) uygulamalar için boş döner.
    Q_INVOKABLE QString command(const QString &name) const;
    // Wayland app_id'sine karşılık gelen uygulama adı. Listede yoksa app_id'nin kendisi döner.
    Q_INVOKABLE QString nameForAppId(const QString &appId) const;

    bool load(const QString &path);

signals:
    void changed();

private:
    void reload();

    QString m_systemPath;
    QString m_userPath;
    QFileSystemWatcher m_watcher;
    QStringList m_names;
    QStringList m_pinned;
    QHash<QString, QString> m_commands;
    QHash<QString, QString> m_namesByAppId;
};
