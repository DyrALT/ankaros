#pragma once

// cursor-shape-v1: uygulama imleci kendi çizmek yerine şeklini söyler ("metin", "el"). Masaüstünün
// imleci Qt çizer, uygulamanın istediği şekil odaktaki yüzeyin öğesine verilir
// (QQuickItem::setCursor). wl_pointer.set_cursor ile gelen resimler çizilmez: boş istek imleci
// gizler, doluysa ok görünür.
#include "qwayland-server-cursor-shape-v1.h"

#include <QCoreApplication>
#include <QCursor>
#include <QtQml/qqmlregistration.h>
#include <QtWaylandCompositor/QWaylandCompositorExtensionTemplate>
#include <QtWaylandCompositor/QWaylandQuickExtension>

class QWaylandClient;
class QWaylandSurface;

class CursorShapeManager : public QWaylandCompositorExtensionTemplate<CursorShapeManager>,
                           public QtWaylandServer::wp_cursor_shape_manager_v1
{
    Q_OBJECT
    QML_ELEMENT

public:
    // Qt'nin QML'den oluşturulan eklentileri kendini başlatmaz, compositor kurulunca initialize()
    // çağrılsın
    CursorShapeManager() { QCoreApplication::postEvent(this, new QEvent(QEvent::Polish)); }
    void initialize() override;

    // İmleci yalnızca işaretçinin üzerinde olduğu uygulama değiştirebilir
    void setCursorForClient(wl_client *client, const QCursor &cursor);

protected:
    void wp_cursor_shape_manager_v1_destroy(Resource *resource) override;
    void wp_cursor_shape_manager_v1_get_pointer(Resource *resource, uint32_t id,
                                                wl_resource *pointer) override;
    void wp_cursor_shape_manager_v1_get_tablet_tool_v2(Resource *resource, uint32_t id,
                                                       wl_resource *tabletTool) override;
};
