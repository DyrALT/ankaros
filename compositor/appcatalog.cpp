#include "appcatalog.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(lcCatalog, "ankaros.catalog")

AppCatalog::AppCatalog(QObject *parent)
    : QObject(parent)
{
    const QString path =
        qEnvironmentVariable("ANKAROS_APPS", QStringLiteral("/etc/ankaros/apps.json"));
    load(path);
}

bool AppCatalog::load(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qCWarning(lcCatalog) << "Uygulama listesi açılamadı:" << path << file.errorString();
        return false;
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(lcCatalog) << "Uygulama listesi okunamadı:" << path << "konum" << error.offset
                             << error.errorString();
        return false;
    }

    const QJsonArray apps = document.object().value(QLatin1String("apps")).toArray();
    for (const QJsonValue &value : apps) {
        const QJsonObject app = value.toObject();
        const QString name = app.value(QLatin1String("name")).toString();
        if (name.isEmpty() || m_names.contains(name)) {
            qCWarning(lcCatalog) << "Adı boş ya da tekrarlanan uygulama atlandı:" << name;
            continue;
        }
        m_names.append(name);
        if (app.value(QLatin1String("pinned")).toBool())
            m_pinned.append(name);
        const QString command = app.value(QLatin1String("command")).toString();
        if (!command.isEmpty())
            m_commands.insert(name, command);
        for (const QJsonValue &appId : app.value(QLatin1String("appIds")).toArray())
            m_namesByAppId.insert(appId.toString(), name);
    }
    qCInfo(lcCatalog) << m_names.size() << "uygulama okundu:" << path;
    return true;
}

QString AppCatalog::command(const QString &name) const
{
    return m_commands.value(name);
}

QString AppCatalog::nameForAppId(const QString &appId) const
{
    return m_namesByAppId.value(appId, appId);
}
