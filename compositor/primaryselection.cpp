#include "primaryselection.h"

#include <QStringList>
#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSeat>
#include <QtWaylandCompositor/QWaylandSurface>

#include <unistd.h>

class PrimarySelectionSource : public QtWaylandServer::zwp_primary_selection_source_v1
{
public:
    PrimarySelectionSource(PrimarySelectionManager *manager, wl_client *client, uint32_t id,
                           int version)
        : zwp_primary_selection_source_v1(client, id, version)
        , m_manager(manager)
    {}

    const QStringList &mimeTypes() const { return m_mimeTypes; }
    wl_client *waylandClient() const { return resource()->client(); }

    void transfer(const QString &mimeType, int fd) { send_send(mimeType, fd); }
    void cancel() { send_cancelled(); }

protected:
    void zwp_primary_selection_source_v1_destroy_resource(Resource *) override
    {
        m_manager->sourceDestroyed(this, m_destroyedByRequest);
        delete this;
    }
    void zwp_primary_selection_source_v1_destroy(Resource *resource) override
    {
        m_destroyedByRequest = true;
        wl_resource_destroy(resource->handle);
    }
    void zwp_primary_selection_source_v1_offer(Resource *, const QString &mimeType) override
    {
        m_mimeTypes.append(mimeType);
    }

private:
    PrimarySelectionManager *m_manager;
    QStringList m_mimeTypes;
    bool m_destroyedByRequest = false;
};

namespace {

class PrimarySelectionOffer : public QtWaylandServer::zwp_primary_selection_offer_v1
{
public:
    PrimarySelectionOffer(PrimarySelectionManager *manager, wl_client *client, int version)
        : zwp_primary_selection_offer_v1(client, 0, version)
        , m_manager(manager)
        , m_generation(manager->generation())
    {}

    void announce(const QStringList &mimeTypes)
    {
        for (const QString &mimeType : mimeTypes)
            send_offer(mimeType);
    }

protected:
    void zwp_primary_selection_offer_v1_destroy_resource(Resource *) override { delete this; }
    void zwp_primary_selection_offer_v1_destroy(Resource *resource) override
    {
        wl_resource_destroy(resource->handle);
    }
    void zwp_primary_selection_offer_v1_receive(Resource *resource, const QString &mimeType,
                                                int32_t fd) override
    {
        // Teklif odak gidince ya da seçim değişince biter, arka plandaki uygulama seçimi okuyamaz
        if (m_generation == m_manager->generation() && m_manager->source() &&
            resource->client() == m_manager->focusedClient())
            m_manager->source()->transfer(mimeType, fd);
        // Olay gönderilirken tanıtıcının kopyası alınır, buradaki kapatılmalı. Teklif geçersizse
        // okuyan taraf boş veri görür.
        close(fd);
    }

private:
    PrimarySelectionManager *m_manager;
    quint64 m_generation;
};

} // namespace

class PrimarySelectionDevice : public QtWaylandServer::zwp_primary_selection_device_v1
{
public:
    PrimarySelectionDevice(PrimarySelectionManager *manager, wl_client *client, uint32_t id,
                           int version)
        : zwp_primary_selection_device_v1(client, id, version)
        , m_manager(manager)
    {}

    wl_client *waylandClient() const { return resource()->client(); }

    void sendCurrent(PrimarySelectionSource *source)
    {
        if (!source) {
            send_selection(nullptr);
            return;
        }
        auto *offer = new PrimarySelectionOffer(m_manager, waylandClient(), resource()->version());
        send_data_offer(offer->resource()->handle);
        offer->announce(source->mimeTypes());
        send_selection(offer->resource()->handle);
    }

protected:
    void zwp_primary_selection_device_v1_destroy_resource(Resource *) override
    {
        m_manager->removeDevice(this);
        delete this;
    }
    void zwp_primary_selection_device_v1_destroy(Resource *resource) override
    {
        wl_resource_destroy(resource->handle);
    }
    void zwp_primary_selection_device_v1_set_selection(Resource *resource, wl_resource *source,
                                                       uint32_t) override
    {
        using SourceProtocol = QtWaylandServer::zwp_primary_selection_source_v1;
        auto *object = source ? SourceProtocol::Resource::fromResource(source)
                                    ->zwp_primary_selection_source_v1_object
                              : nullptr;
        m_manager->setSelection(resource->client(), static_cast<PrimarySelectionSource *>(object));
    }

private:
    PrimarySelectionManager *m_manager;
};

void PrimarySelectionManager::initialize()
{
    QWaylandCompositorExtensionTemplate::initialize();
    auto *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    init(compositor->display(), 1);
    connect(compositor->defaultSeat(), &QWaylandSeat::keyboardFocusChanged, this,
            [this](QWaylandSurface *focus, QWaylandSurface *previous) {
                // Odaktaki yüzey yok olunca Qt odağı sinyalsiz sıfırlar, previous o zaman boştur
                if (!previous || !focus || previous->waylandClient() != focus->waylandClient())
                    announce();
            });
}

wl_client *PrimarySelectionManager::focusedClient() const
{
    auto *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    const QWaylandSurface *focus = compositor->defaultSeat()->keyboardFocus();
    return focus ? focus->waylandClient() : nullptr;
}

void PrimarySelectionManager::announce(wl_client *except)
{
    wl_client *focused = focusedClient();
    if (!focused || focused == except)
        return;
    for (PrimarySelectionDevice *device : std::as_const(m_devices)) {
        if (device->waylandClient() == focused)
            device->sendCurrent(m_source);
    }
}

void PrimarySelectionManager::setSelection(wl_client *client, PrimarySelectionSource *source)
{
    // Arka plandaki uygulama seçimi değiştiremez
    if (client != focusedClient() || source == m_source)
        return;
    if (m_source)
        m_source->cancel();
    m_source = source;
    ++m_generation;
    announce();
}

void PrimarySelectionManager::sendSelection(PrimarySelectionDevice *device)
{
    if (device->waylandClient() == focusedClient())
        device->sendCurrent(m_source);
}

void PrimarySelectionManager::removeDevice(PrimarySelectionDevice *device)
{
    m_devices.removeOne(device);
}

void PrimarySelectionManager::sourceDestroyed(PrimarySelectionSource *source,
                                              bool destroyedByRequest)
{
    if (source != m_source)
        return;
    m_source = nullptr;
    ++m_generation;
    // Uygulama kapanıyorsa ona yeni nesne oluşturulmamalı
    announce(destroyedByRequest ? nullptr : source->waylandClient());
}

void PrimarySelectionManager::zwp_primary_selection_device_manager_v1_create_source(
    Resource *resource, uint32_t id)
{
    new PrimarySelectionSource(this, resource->client(), id, resource->version());
}

void PrimarySelectionManager::zwp_primary_selection_device_manager_v1_get_device(Resource *resource,
                                                                                 uint32_t id,
                                                                                 wl_resource *)
{
    auto *device = new PrimarySelectionDevice(this, resource->client(), id, resource->version());
    m_devices.append(device);
    sendSelection(device);
}

void PrimarySelectionManager::zwp_primary_selection_device_manager_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}
