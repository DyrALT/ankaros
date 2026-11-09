#pragma once

#include <QtQml/qqmlregistration.h>
#include <QtWaylandCompositor/private/qwaylandcompositorquickextensions_p.h>

class SpiceAgent;

// WaylandCompositor'ın kendisi, tek farkla: uygulamalardan birinin panoya koyduğu metni görebilir
// ve panoyu kendisi değiştirebilir. Sanal makinede ekranı açan programla (UTM) pano paylaşımı
// bunu kullanır (SpiceAgent, docs/pano.md). SPICE kanalı yoksa hiçbir şey değişmez.
// QML'deki WaylandCompositor tipi bu sınıftır (içine yazılan WaylandOutput, XdgShell gibi
// eklentileri o toplar), bu yüzden QWaylandQuickCompositor'dan değil ondan türer.
class AnkarosCompositor : public QWaylandQuickCompositorQuickExtensionContainer
{
    Q_OBJECT
    QML_ELEMENT

public:
    explicit AnkarosCompositor(QObject *parent = nullptr);

    void componentComplete() override;

protected:
    void retainedSelectionReceived(QMimeData *mimeData) override;

private:
    void setHostText(const QString &text);
    void injectMouse(int x, int y, quint32 buttons, int display);

    SpiceAgent *m_spice = nullptr;
    bool m_overriding = false;
    quint32 m_spiceButtons = 0;
};
