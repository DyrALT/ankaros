#include "ankaroscompositor.h"

#include "spiceagent.h"

#include <QCoreApplication>
#include <QGuiApplication>
#include <QMimeData>
#include <QScreen>
#include <QWindow>
#include <qpa/qplatformscreen.h>
#include <qpa/qplatformwindow.h>
#include <qpa/qwindowsysteminterface.h>

namespace {

// spice/vd_agent.h: VD_AGENT_LBUTTON = 1, MBUTTON, RBUTTON, UBUTTON, DBUTTON, SBUTTON, EBUTTON
constexpr quint32 buttonMask(int button) { return 1u << button; }

const struct
{
    quint32 mask;
    Qt::MouseButton button;
} spiceButtons[] = {
    {buttonMask(1), Qt::LeftButton},
    {buttonMask(2), Qt::MiddleButton},
    {buttonMask(3), Qt::RightButton},
    {buttonMask(6), Qt::BackButton},
    {buttonMask(7), Qt::ForwardButton},
};
constexpr quint32 wheelUp = buttonMask(4);
constexpr quint32 wheelDown = buttonMask(5);

} // namespace

AnkarosCompositor::AnkarosCompositor(QObject *parent)
{
    setParent(parent);
}

void AnkarosCompositor::componentComplete()
{
    QWaylandQuickCompositorQuickExtensionContainer::componentComplete();
    // Giriş ekranında pano paylaşılmaz: oturum açmamış birinin Mac'in panosunu okuması gerekmez
    if (QCoreApplication::arguments().contains(QStringLiteral("--login")))
        return;
    m_spice = SpiceAgent::open(this);
    if (!m_spice)
        return;
    // Uygulama panoya bir şey koyunca compositor içeriği okur (retainedSelectionReceived). Yan
    // etkisi: kopyalanan metin, kopyalayan uygulama kapansa da panoda kalır.
    setRetainedSelectionEnabled(true);
    connect(m_spice, &SpiceAgent::hostTextReceived, this, &AnkarosCompositor::setHostText);
    // Pano paylaşımı açıkken UTM fareyi ajan kanalından gönderir, sanal tablet sessiz kalır
    connect(m_spice, &SpiceAgent::mouseStateReceived, this, &AnkarosCompositor::injectMouse);
    qInfo("pano: SPICE kanalı açıldı, pano sanal makineyi açan programla paylaşılıyor");
}

void AnkarosCompositor::retainedSelectionReceived(QMimeData *mimeData)
{
    // overrideSelection de bu işlevi çağırır: Mac'ten gelen metin Mac'e geri gönderilmez
    if (m_overriding || !m_spice || !mimeData)
        return;
    QString text;
    if (mimeData->hasFormat(QStringLiteral("text/plain;charset=utf-8")))
        text = QString::fromUtf8(mimeData->data(QStringLiteral("text/plain;charset=utf-8")));
    else if (mimeData->hasText())
        text = mimeData->text();
    m_spice->setGuestText(text);
}

void AnkarosCompositor::setHostText(const QString &text)
{
    QMimeData data;
    const QByteArray utf8 = text.toUtf8();
    data.setData(QStringLiteral("text/plain;charset=utf-8"), utf8);
    data.setData(QStringLiteral("text/plain"), utf8);
    // X11 uygulamaları (xwayland-satellite) ve eski GTK uygulamaları bu adları sorar
    data.setData(QStringLiteral("UTF8_STRING"), utf8);
    data.setData(QStringLiteral("TEXT"), utf8);
    m_overriding = true;
    overrideSelection(&data);
    m_overriding = false;
}

void AnkarosCompositor::injectMouse(int x, int y, quint32 buttons, int display)
{
    const QList<QScreen *> screens = QGuiApplication::screens();
    QScreen *screen = screens.value(display, QGuiApplication::primaryScreen());
    if (!screen)
        return;
    // SPICE ekranın piksellerini verir. QWindowSystemInterface de ekran pikseli (native) bekler.
    const QPoint global = screen->handle()->geometry().topLeft() + QPoint(x, y);
    QWindow *window = nullptr;
    for (QWindow *candidate : QGuiApplication::topLevelWindows()) {
        if (candidate->isVisible() && candidate->screen() == screen && candidate->handle()) {
            window = candidate;
            break;
        }
    }
    if (!window)
        return;
    const QPointF local = global - window->handle()->geometry().topLeft();
    const Qt::KeyboardModifiers modifiers = QGuiApplication::keyboardModifiers();

    Qt::MouseButtons state;
    for (const auto &entry : spiceButtons) {
        if (m_spiceButtons & entry.mask)
            state |= entry.button;
    }
    // Önce konum, sonra her değişen tuş için ayrı basma/bırakma olayı
    QWindowSystemInterface::handleMouseEvent(window, local, QPointF(global), state, Qt::NoButton,
                                             QEvent::MouseMove, modifiers);
    for (const auto &entry : spiceButtons) {
        if (((m_spiceButtons ^ buttons) & entry.mask) == 0)
            continue;
        const bool pressed = buttons & entry.mask;
        state.setFlag(entry.button, pressed);
        QWindowSystemInterface::handleMouseEvent(
                window, local, QPointF(global), state, entry.button,
                pressed ? QEvent::MouseButtonPress : QEvent::MouseButtonRelease, modifiers);
    }
    // Tekerlek: SPICE her çentik için basıp bırakır
    const quint32 newlyPressed = buttons & ~m_spiceButtons;
    if (newlyPressed & (wheelUp | wheelDown)) {
        const int delta = (newlyPressed & wheelUp) ? 120 : -120;
        QWindowSystemInterface::handleWheelEvent(window, local, QPointF(global), QPoint(),
                                                 QPoint(0, delta), modifiers);
    }
    m_spiceButtons = buttons;
}
