#pragma once

// xdg-activation-v1: bir uygulama başka bir pencereyi öne getirmek ister (ör. terminaldeki bağlantı
// tarayıcıyı açar). Jeton yalnızca klavye odağı kendisindeyken ve kullanıcı az önce bir tuşa ya da
// fareye bastıysa geçerlidir, uygulama kendiliğinden odak çalamaz.
#include "qwayland-server-xdg-activation-v1.h"

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QHash>
#include <QString>
#include <QtQml/qqmlregistration.h>
#include <QtWaylandCompositor/QWaylandCompositorExtensionTemplate>
#include <QtWaylandCompositor/QWaylandQuickExtension>
#include <QtWaylandCompositor/QWaylandSurface>

class XdgActivationManager : public QWaylandCompositorExtensionTemplate<XdgActivationManager>,
                             public QtWaylandServer::xdg_activation_v1
{
    Q_OBJECT
    QML_ELEMENT

public:
    // Qt'nin QML'den oluşturulan eklentileri kendini başlatmaz, compositor kurulunca initialize()
    // çağrılsın
    XdgActivationManager();
    void initialize() override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    // Jeton üretir. Yalnızca odaktaki uygulamanın kendi yüzeyiyle istediği jeton kullanılabilir,
    // ötekiler de bir jeton alır (protokol gereği) ama işe yaramaz.
    QString createToken(wl_client *client, QWaylandSurface *surface);

signals:
    void activationRequested(QWaylandSurface *surface);

protected:
    void xdg_activation_v1_destroy(Resource *resource) override;
    void xdg_activation_v1_get_activation_token(Resource *resource, uint32_t id) override;
    void xdg_activation_v1_activate(Resource *resource, const QString &token,
                                    wl_resource *surface) override;

private:
    QHash<QString, QElapsedTimer> m_tokens;
    QElapsedTimer m_lastInput;
};
