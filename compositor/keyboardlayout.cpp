#include "keyboardlayout.h"

#include <QGuiApplication>
#include <QKeyEvent>
#include <QWindow>
#include <QtGui/private/qxkbcommon_p.h>
#include <QtWaylandCompositor/QWaylandQuickItem>

#include <xkbcommon/xkbcommon.h>

namespace {

// "tr-f" -> ("tr", "f")
QString layoutPart(const QString &layout)
{
    return layout.section(QLatin1Char('-'), 0, 0);
}
QString variantPart(const QString &layout)
{
    return layout.section(QLatin1Char('-'), 1);
}

} // namespace

KeyboardLayout::KeyboardLayout(QObject *parent)
    : QObject(parent)
    , m_context(xkb_context_new(XKB_CONTEXT_NO_FLAGS))
{
    const QString layout = qEnvironmentVariable("XKB_DEFAULT_LAYOUT", QStringLiteral("tr"));
    const QString variant = qEnvironmentVariable("XKB_DEFAULT_VARIANT");
    m_startupLayout = variant.isEmpty() ? layout : layout + QLatin1Char('-') + variant;
    m_layout = m_startupLayout;
    qApp->installEventFilter(this);
}

KeyboardLayout::~KeyboardLayout()
{
    freeKeymap();
    xkb_context_unref(m_context);
}

void KeyboardLayout::freeKeymap()
{
    xkb_state_unref(m_state);
    xkb_keymap_unref(m_keymap);
    m_state = nullptr;
    m_keymap = nullptr;
}

QString KeyboardLayout::xkbLayout() const
{
    return layoutPart(m_layout);
}

QString KeyboardLayout::xkbVariant() const
{
    return variantPart(m_layout);
}

void KeyboardLayout::setLayout(const QString &layout)
{
    if (layout.isEmpty() || layout == m_layout)
        return;
    m_layout = layout;
    freeKeymap();
    if (m_layout != m_startupLayout && m_context) {
        const QByteArray name = layoutPart(m_layout).toLatin1();
        const QByteArray variant = variantPart(m_layout).toLatin1();
        const xkb_rule_names names = {"evdev", "pc105", name.constData(), variant.constData(), ""};
        m_keymap = xkb_keymap_new_from_names(m_context, &names, XKB_KEYMAP_COMPILE_NO_FLAGS);
        if (m_keymap)
            m_state = xkb_state_new(m_keymap);
        else
            qWarning("Klavye düzeni yüklenemedi: %s", qPrintable(m_layout));
    }
    emit layoutChanged();
}

bool KeyboardLayout::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_state || m_sending ||
        (event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease))
        return false;
    auto *window = qobject_cast<QWindow *>(watched);
    if (!window)
        return false;

    const auto *key = static_cast<QKeyEvent *>(event);
    const xkb_keycode_t code = key->nativeScanCode();
    if (code == 0)
        return false;
    const bool pressed = event->type() == QEvent::KeyPress;

    // Qt'nin libinput klavyesi gibi: metin ve tuş, değiştirici tuşların olaydan önceki haliyle
    const xkb_keysym_t sym = xkb_state_key_get_one_sym(m_state, code);
    const Qt::KeyboardModifiers modifiers = QXkbCommon::modifiers(m_state);
    const QString text = QXkbCommon::lookupString(m_state, code);
    const int qtKey = QXkbCommon::keysymToQtKey(sym, modifiers, m_state, code);
    if (!key->isAutoRepeat())
        xkb_state_update_key(m_state, code, pressed ? XKB_KEY_DOWN : XKB_KEY_UP);

    // Odak bir uygulamanın penceresindeyse dokunma: uygulama tuşu Wayland klavye haritasıyla
    // çevirir
    if (qobject_cast<QWaylandQuickItem *>(window->focusObject()))
        return false;

    QKeyEvent translated(event->type(), qtKey, modifiers, code, sym, key->nativeModifiers(), text,
                         key->isAutoRepeat(), key->count());
    m_sending = true;
    QCoreApplication::sendEvent(window, &translated);
    m_sending = false;
    return true;
}
