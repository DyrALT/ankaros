#include "ankaroscompositor.h"

#include "spiceagent.h"

#include <QCoreApplication>
#include <QMimeData>

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
