#pragma once

// Qt'de wp_viewporter var ama QML'e açılmamış. Chromium ve Electron bu protokol olmadan açılmaz.
#include <QtQml/qqmlregistration.h>
#include <QtWaylandCompositor/QWaylandViewporter>

struct ViewporterForeign
{
    Q_GADGET
    QML_FOREIGN(QWaylandViewporter)
    QML_NAMED_ELEMENT(Viewporter)
};
