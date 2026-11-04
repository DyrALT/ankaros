#pragma once

// primary-selection-unstable-v1: fareyle seçilen metin orta tuşla yapıştırılır. Pano gibi çalışır,
// ayrı bir seçimdir: seçimi odaktaki uygulama belirler, yalnızca odaktaki uygulamaya duyurulur.
// Veri doğrudan iki uygulama arasında akar, compositor yalnızca dosya tanıtıcısını iletir.
#include "qwayland-server-primary-selection-unstable-v1.h"

#include <QCoreApplication>
#include <QList>
#include <QtQml/qqmlregistration.h>
#include <QtWaylandCompositor/QWaylandCompositorExtensionTemplate>
#include <QtWaylandCompositor/QWaylandQuickExtension>

class QWaylandSurface;
class PrimarySelectionDevice;
class PrimarySelectionSource;

class PrimarySelectionManager : public QWaylandCompositorExtensionTemplate<PrimarySelectionManager>,
                                public QtWaylandServer::zwp_primary_selection_device_manager_v1
{
    Q_OBJECT
    QML_ELEMENT

public:
    // Qt'nin QML'den oluşturulan eklentileri kendini başlatmaz, compositor kurulunca initialize()
    // çağrılsın
    PrimarySelectionManager() { QCoreApplication::postEvent(this, new QEvent(QEvent::Polish)); }
    void initialize() override;

    void setSelection(wl_client *client, PrimarySelectionSource *source);
    void sendSelection(PrimarySelectionDevice *device);
    void removeDevice(PrimarySelectionDevice *device);
    void sourceDestroyed(PrimarySelectionSource *source);

    // Seçim her değiştiğinde artar, eski teklifler böylece geçersiz kalır
    quint64 generation() const { return m_generation; }
    PrimarySelectionSource *source() const { return m_source; }

protected:
    void zwp_primary_selection_device_manager_v1_create_source(Resource *resource,
                                                               uint32_t id) override;
    void zwp_primary_selection_device_manager_v1_get_device(Resource *resource, uint32_t id,
                                                            wl_resource *seat) override;
    void zwp_primary_selection_device_manager_v1_destroy(Resource *resource) override;

private:
    wl_client *focusedClient() const;
    void focusChanged(QWaylandSurface *focus);
    void announce(wl_client *except = nullptr);

    QList<PrimarySelectionDevice *> m_devices;
    PrimarySelectionSource *m_source = nullptr;
    wl_client *m_focusedClient = nullptr;
    quint64 m_generation = 0;
};
