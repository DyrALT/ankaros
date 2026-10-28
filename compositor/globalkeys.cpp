#include "globalkeys.h"

#include <QGuiApplication>
#include <QKeyEvent>

namespace {

bool isSuper(int key)
{
    return key == Qt::Key_Meta || key == Qt::Key_Super_L || key == Qt::Key_Super_R;
}

} // namespace

GlobalKeys::GlobalKeys(QObject *parent)
    : QObject(parent)
{
    qApp->installEventFilter(this);
}

void GlobalKeys::setEnabled(bool enabled)
{
    if (m_enabled == enabled)
        return;
    m_enabled = enabled;
    m_superDown = false;
    setSwitching(false);
    emit enabledChanged();
}

void GlobalKeys::setSwitching(bool switching)
{
    if (m_switching == switching)
        return;
    m_switching = switching;
    emit switchingChanged();
}

bool GlobalKeys::eventFilter(QObject *watched, QEvent *event)
{
    if (!m_enabled)
        return false;

    switch (event->type()) {
    case QEvent::ShortcutOverride: {
        // Süper+D gibi bir kısayol çalışacaksa tuş olayı pencereye hiç ulaşmaz, yalnızca bu gelir.
        // Odaktaki öğeye gönderildiği için pencere süzgecinin dışında bakılır.
        const auto *key = static_cast<QKeyEvent *>(event);
        if (m_superDown && !isSuper(key->key()))
            m_superUsed = true;
        return false;
    }
    case QEvent::MouseButtonPress:
        if (m_superDown)
            m_superUsed = true;
        return false;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
        break;
    default:
        return false;
    }

    // Tuş olayları önce pencereye, sonra odaktaki öğeye (uygulamanın yüzeyi) gönderilir. Yalnızca
    // pencerede bakılır, burada tüketilen tuş uygulamaya gitmez.
    if (!watched->isWindowType())
        return false;

    const auto *key = static_cast<QKeyEvent *>(event);
    if (event->type() == QEvent::KeyPress) {
        if (isSuper(key->key())) {
            if (!key->isAutoRepeat()) {
                m_superDown = true;
                m_superUsed = false;
            }
            return false;
        }
        if (m_superDown)
            m_superUsed = true;

        const bool tab = key->key() == Qt::Key_Tab || key->key() == Qt::Key_Backtab;
        if (tab && (key->modifiers() & Qt::AltModifier)) {
            setSwitching(true);
            emit switchRequested(key->key() == Qt::Key_Backtab ||
                                 (key->modifiers() & Qt::ShiftModifier));
            return true;
        }
        if (m_switching && key->key() == Qt::Key_Escape) {
            setSwitching(false);
            emit switchCancelled();
            return true;
        }
        return false;
    }

    // KeyRelease
    if (m_switching && key->key() == Qt::Key_Alt) {
        setSwitching(false);
        emit switchCommitted();
        return false;
    }
    if (isSuper(key->key()) && !key->isAutoRepeat()) {
        if (m_superDown && !m_superUsed)
            emit superTapped();
        m_superDown = false;
    }
    return false;
}
