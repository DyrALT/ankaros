#include "xdgactivation.h"

#include <QPointer>
#include <QUuid>
#include <QtWaylandCompositor/QWaylandCompositor>
#include <QtWaylandCompositor/QWaylandSeat>
#include <QtWaylandCompositor/QWaylandSurface>

namespace {

// Jeton kısa ömürlüdür: tıklamadan pencerenin açılmasına kadar geçen süre
constexpr qint64 tokenLifetimeMs = 30'000;

class ActivationToken : public QtWaylandServer::xdg_activation_token_v1
{
public:
    ActivationToken(XdgActivationManager *manager, wl_client *client, uint32_t id, int version)
        : xdg_activation_token_v1(client, id, version)
        , m_manager(manager)
    {}

protected:
    void xdg_activation_token_v1_destroy_resource(Resource *) override { delete this; }
    void xdg_activation_token_v1_destroy(Resource *resource) override
    {
        wl_resource_destroy(resource->handle);
    }
    void xdg_activation_token_v1_set_serial(Resource *, uint32_t, wl_resource *) override {}
    void xdg_activation_token_v1_set_app_id(Resource *, const QString &) override {}
    void xdg_activation_token_v1_set_surface(Resource *resource, wl_resource *surface) override
    {
        if (m_committed) {
            wl_resource_post_error(resource->handle, error_already_used, "jeton zaten istendi");
            return;
        }
        m_surface = QWaylandSurface::fromResource(surface);
    }
    void xdg_activation_token_v1_commit(Resource *resource) override
    {
        if (m_committed) {
            wl_resource_post_error(resource->handle, error_already_used, "jeton zaten istendi");
            return;
        }
        m_committed = true;
        send_done(resource->handle, m_manager->createToken(resource->client(), m_surface));
    }

private:
    XdgActivationManager *m_manager;
    QPointer<QWaylandSurface> m_surface;
    bool m_committed = false;
};

} // namespace

void XdgActivationManager::initialize()
{
    QWaylandCompositorExtensionTemplate::initialize();
    init(static_cast<QWaylandCompositor *>(extensionContainer())->display(), 1);
}

QString XdgActivationManager::createToken(wl_client *client, QWaylandSurface *surface)
{
    const QString token = QUuid::createUuid().toString(QUuid::WithoutBraces);
    auto *compositor = static_cast<QWaylandCompositor *>(extensionContainer());
    const QWaylandSurface *focus = compositor->defaultSeat()->keyboardFocus();
    if (!surface || surface->waylandClient() != client || !focus ||
        focus->waylandClient() != client)
        return token;

    m_tokens.removeIf([](QHash<QString, QElapsedTimer>::iterator entry) {
        return entry->hasExpired(tokenLifetimeMs);
    });
    QElapsedTimer created;
    created.start();
    m_tokens.insert(token, created);
    return token;
}

void XdgActivationManager::xdg_activation_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void XdgActivationManager::xdg_activation_v1_get_activation_token(Resource *resource, uint32_t id)
{
    new ActivationToken(this, resource->client(), id, resource->version());
}

void XdgActivationManager::xdg_activation_v1_activate(Resource *, const QString &token,
                                                      wl_resource *surface)
{
    // Geçersiz jeton hata değildir, protokol isteğin sessizce yok sayılmasına izin verir
    const auto entry = m_tokens.constFind(token);
    if (entry == m_tokens.cend())
        return;
    const bool valid = !entry->hasExpired(tokenLifetimeMs);
    m_tokens.erase(entry);
    if (valid)
        emit activationRequested(QWaylandSurface::fromResource(surface));
}
