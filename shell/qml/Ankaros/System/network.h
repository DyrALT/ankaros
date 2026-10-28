#pragma once

#include <QHash>
#include <QObject>
#include <QSocketNotifier>
#include <QString>
#include <QTimer>
#include <QtQml/qqmlregistration.h>

// Kablolu ağın durumu ve ayarı (docs/ag.md). ankaros-network'ün soketine abone olur, durum her
// değiştiğinde özellikler güncellenir. Kabuk bunu Desktop.network olarak kullanır (alt çubuktaki
// simge, Ayarlar › Ağ).
class Network : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

    // Ağ servisine bağlı ve durum biliniyor mu
    Q_PROPERTY(bool available READ available NOTIFY changed)
    Q_PROPERTY(QString interfaceName READ interfaceName NOTIFY changed)
    // Kablo takılı mı
    Q_PROPERTY(bool carrier READ carrier NOTIFY changed)
    // Kablo takılı ve bir IPv4 adresi var
    Q_PROPERTY(bool connected READ connected NOTIFY changed)
    // "dhcp" ya da "static"
    Q_PROPERTY(QString mode READ mode NOTIFY changed)
    Q_PROPERTY(QString address READ address NOTIFY changed)
    Q_PROPERTY(QString gateway READ gateway NOTIFY changed)
    Q_PROPERTY(QString dns READ dns NOTIFY changed)
    Q_PROPERTY(QString mac READ mac NOTIFY changed)
    // network.conf'taki statik ayar (elle girilen değerler)
    Q_PROPERTY(QString configAddress READ configAddress NOTIFY changed)
    Q_PROPERTY(QString configGateway READ configGateway NOTIFY changed)
    Q_PROPERTY(QString configDns READ configDns NOTIFY changed)

public:
    explicit Network(QObject *parent = nullptr);
    ~Network() override;

    bool available() const { return m_available; }
    QString interfaceName() const { return value("interface"); }
    bool carrier() const { return value("carrier") == QLatin1String("1"); }
    bool connected() const { return carrier() && !address().isEmpty(); }
    QString mode() const { return value("mode"); }
    QString address() const { return value("address"); }
    QString gateway() const { return value("gateway"); }
    QString dns() const { return value("dns"); }
    QString mac() const { return value("mac"); }
    QString configAddress() const { return value("config.address"); }
    QString configGateway() const { return value("config.gateway"); }
    QString configDns() const { return value("config.dns"); }

    // mode "dhcp" ya da "static". dns boşlukla ayrılmış adresler.
    Q_INVOKABLE void apply(const QString &mode, const QString &address, const QString &gateway,
                           const QString &dns);

signals:
    void changed();
    void applied();
    void applyFailed(const QString &message);

private:
    QString value(const char *key) const { return m_values.value(QString::fromLatin1(key)); }
    void connectToService();
    void disconnectFromService();
    void readUpdates();

    int m_fd = -1;
    QSocketNotifier *m_notifier = nullptr;
    QTimer m_reconnect;
    QByteArray m_buffer;
    QHash<QString, QString> m_values;
    bool m_available = false;
};
