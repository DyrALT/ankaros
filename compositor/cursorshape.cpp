#include "cursorshape.h"

#include <QQuickItem>
#include <QtWaylandCompositor/QWaylandClient>
#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSeat>
#include <QtWaylandCompositor/QWaylandSurface>
#include <QtWaylandCompositor/QWaylandView>

#include <optional>

namespace {

using Device = QtWaylandServer::wp_cursor_shape_device_v1;

// Sürüm 1'deki şekiller. Qt'de karşılığı olmayanlar en yakın şekle düşer.
std::optional<Qt::CursorShape> qtShape(uint32_t shape)
{
    switch (shape) {
    case Device::shape_default:
    case Device::shape_context_menu:
    case Device::shape_zoom_in:
    case Device::shape_zoom_out:
        return Qt::ArrowCursor;
    case Device::shape_help:
        return Qt::WhatsThisCursor;
    case Device::shape_pointer:
        return Qt::PointingHandCursor;
    case Device::shape_progress:
        return Qt::BusyCursor;
    case Device::shape_wait:
        return Qt::WaitCursor;
    case Device::shape_cell:
    case Device::shape_crosshair:
        return Qt::CrossCursor;
    case Device::shape_text:
    case Device::shape_vertical_text:
        return Qt::IBeamCursor;
    case Device::shape_alias:
        return Qt::DragLinkCursor;
    case Device::shape_copy:
        return Qt::DragCopyCursor;
    case Device::shape_move:
        return Qt::DragMoveCursor;
    case Device::shape_no_drop:
    case Device::shape_not_allowed:
        return Qt::ForbiddenCursor;
    case Device::shape_grab:
        return Qt::OpenHandCursor;
    case Device::shape_grabbing:
        return Qt::ClosedHandCursor;
    case Device::shape_e_resize:
    case Device::shape_w_resize:
    case Device::shape_ew_resize:
        return Qt::SizeHorCursor;
    case Device::shape_n_resize:
    case Device::shape_s_resize:
    case Device::shape_ns_resize:
        return Qt::SizeVerCursor;
    case Device::shape_ne_resize:
    case Device::shape_sw_resize:
    case Device::shape_nesw_resize:
        return Qt::SizeBDiagCursor;
    case Device::shape_nw_resize:
    case Device::shape_se_resize:
    case Device::shape_nwse_resize:
        return Qt::SizeFDiagCursor;
    case Device::shape_col_resize:
        return Qt::SplitHCursor;
    case Device::shape_row_resize:
        return Qt::SplitVCursor;
    case Device::shape_all_scroll:
        return Qt::SizeAllCursor;
    default:
        return std::nullopt;
    }
}

class CursorShapeDevice : public Device
{
public:
    // pointer boşsa (çizim tableti) nesne vardır ama hiçbir şey yapmaz
    CursorShapeDevice(CursorShapeManager *manager, wl_client *client, uint32_t id, int version,
                      wl_resource *pointer)
        : Device(client, id, version)
        , m_manager(manager)
        , m_inert(!pointer)
    {
        m_pointerGone.device = this;
        m_pointerGone.listener.notify = [](wl_listener *listener, void *) {
            reinterpret_cast<PointerListener *>(listener)->device->m_inert = true;
            wl_list_remove(&listener->link);
            wl_list_init(&listener->link);
        };
        wl_list_init(&m_pointerGone.listener.link);
        if (pointer)
            wl_resource_add_destroy_listener(pointer, &m_pointerGone.listener);
    }
    ~CursorShapeDevice() override { wl_list_remove(&m_pointerGone.listener.link); }

protected:
    void wp_cursor_shape_device_v1_destroy_resource(Resource *) override { delete this; }
    void wp_cursor_shape_device_v1_destroy(Resource *resource) override
    {
        wl_resource_destroy(resource->handle);
    }
    void wp_cursor_shape_device_v1_set_shape(Resource *resource, uint32_t, uint32_t shape) override
    {
        const std::optional<Qt::CursorShape> cursor = qtShape(shape);
        if (!cursor) {
            wl_resource_post_error(resource->handle, error_invalid_shape, "bilinmeyen imleç şekli");
            return;
        }
        if (!m_inert)
            m_manager->setCursorForClient(resource->client(), *cursor);
    }

private:
    struct PointerListener
    {
        wl_listener listener; // İlk üye olmalı: bildirimden nesneye dönülüyor
        CursorShapeDevice *device;
    };

    CursorShapeManager *m_manager;
    PointerListener m_pointerGone;
    bool m_inert;
};

} // namespace

void CursorShapeManager::initialize()
{
    QWaylandCompositorExtensionTemplate::initialize();
    auto *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    // Protokolün 2. sürümündeki şekillerin Qt'de karşılığı yok, sürüm 1 yeterli
    init(compositor->display(), 1);
    connect(compositor->defaultSeat(), &QWaylandSeat::cursorSurfaceRequested, this,
            [this](QWaylandSurface *surface, int, int, QWaylandClient *client) {
                setCursorForClient(client->client(), surface ? Qt::ArrowCursor : Qt::BlankCursor);
            });
}

void CursorShapeManager::setCursorForClient(wl_client *client, const QCursor &cursor)
{
    auto *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    QWaylandView *view = compositor->defaultSeat()->mouseFocus();
    if (!view || !view->surface() || view->surface()->waylandClient() != client)
        return;
    if (auto *item = qobject_cast<QQuickItem *>(view->renderObject()))
        item->setCursor(cursor);
}

void CursorShapeManager::wp_cursor_shape_manager_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void CursorShapeManager::wp_cursor_shape_manager_v1_get_pointer(Resource *resource, uint32_t id,
                                                                wl_resource *pointer)
{
    new CursorShapeDevice(this, resource->client(), id, resource->version(), pointer);
}

void CursorShapeManager::wp_cursor_shape_manager_v1_get_tablet_tool_v2(Resource *resource,
                                                                       uint32_t id, wl_resource *)
{
    // Çizim tableti desteklenmiyor, nesne yine de oluşturulmalı
    new CursorShapeDevice(this, resource->client(), id, resource->version(), nullptr);
}
