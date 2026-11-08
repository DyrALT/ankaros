// Bir Terminal penceresi: sekme çubuğu ve sekmeler. Pencere çerçevesini ankaros'ta compositor çizer
// (karar 0007). Figma: "Faz 3 · Uygulamalar" sayfası, Terminal (açık/koyu).
//
// Sekmeler (TerminalView) pencereye değil uygulamaya aittir (Main.qml): pencereler arasında taşınırken
// içlerinde çalışan kabuk kapanmaz. Sekme sürüklenince sıra değişir, çubuğun dışına bırakılınca yeni
// bir pencereye taşınır.
//
// Kısayollar: Ctrl+Shift+T yeni sekme, Ctrl+Shift+N yeni pencere, Ctrl+Shift+W sekmeyi kapat,
// Ctrl+Tab sonraki sekme, Ctrl+Shift+C kopyala, Ctrl+Shift+V yapıştır, Ctrl+artı/eksi/0 yazı boyutu,
// Shift+PageUp/Down kaydır.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Ankaros.UI

Window {
    id: window

    // Uygulama (Main.qml): ayarlar, renkler, sekme ve pencere oluşturma
    required property var app
    // Boş değilse açılıştan kısa süre sonra pencerenin görüntüsü bu dosyaya kaydedilir ve uygulama kapanır
    property string screenshot
    // Sekmelerin terminalleri, çubuktaki sırayla
    property list<TerminalView> views
    property int current: 0
    readonly property TerminalView currentView: current < views.length ? views[current] : null
    // Sekme çubuğunun yüksekliği. Sekme bunun dışına bu kadar sürüklenip bırakılırsa yeni pencereye taşınır.
    readonly property int tabBarHeight: 35
    readonly property int detachDistance: 40

    // Bir terminali bu pencereye sekme olarak ekler ve seçer
    function addView(view: TerminalView, index: int) {
        view.parent = stack;
        view.visible = Qt.binding(() => window.currentView === view);
        view.focus = Qt.binding(() => window.currentView === view);
        const list = Array.from(views);
        list.splice(index < 0 ? list.length : index, 0, view);
        views = list;
        current = list.indexOf(view);
    }

    // Terminali sekmelerden çıkarır (yok etmez). Son sekme çıkarsa pencere kapanır.
    function takeView(view: TerminalView) {
        const index = views.indexOf(view);
        if (index < 0)
            return;
        const list = Array.from(views);
        list.splice(index, 1);
        views = list;
        if (list.length === 0) {
            app.windowClosed(window);
            return;
        }
        if (index < current || current >= list.length)
            current = current - 1;
    }

    function newTab() {
        addView(app.createView(stack), -1);
    }

    function closeTab(index: int) {
        const view = views[index];
        takeView(view);
        view.destroy();
    }

    function moveTab(from: int, to: int) {
        if (from === to)
            return;
        const view = views[from];
        const list = Array.from(views);
        list.splice(from, 1);
        list.splice(to, 0, view);
        views = list;
        current = to;
    }

    // Sürüklenen sekme bırakıldı. point: sekme çubuğundaki satıra (tabRow) göre bırakılan yer.
    function dropTab(index: int, point: point) {
        const outside = point.y < -detachDistance || point.y > tabBarHeight + detachDistance
                        || point.x < -detachDistance || point.x > width + detachDistance;
        if (outside) {
            // Tek sekmeyse yeni pencere eskisinin aynısı olurdu
            if (views.length > 1) {
                const view = views[index];
                takeView(view);
                app.newWindow(view, Qt.size(width, height));
            }
            return;
        }
        // Ortası bırakılan yerin solunda kalan sekmeler kadar ileri
        let target = 0;
        for (let i = 0; i < tabRepeater.count; ++i) {
            const item = tabRepeater.itemAt(i);
            if (i !== index && item && point.x > item.x + item.width / 2)
                ++target;
        }
        moveTab(index, target);
    }

    width: 880
    height: 560
    minimumWidth: 420
    minimumHeight: 240
    title: qsTr("Terminal")
    color: app.colors.background

    onCurrentViewChanged: Qt.callLater(() => window.currentView?.forceActiveFocus())
    // Pencere kapatılınca (başlık çubuğundaki düğme) içindeki sekmeler de kapanır
    onClosing: {
        for (const view of views)
            view.destroy();
        views = [];
        app.windowClosed(window);
    }

    Shortcut {
        sequence: "Ctrl+Shift+T"
        onActivated: window.newTab()
    }

    Shortcut {
        sequence: "Ctrl+Shift+N"
        onActivated: window.app.newWindow(null, Qt.size(window.width, window.height))
    }

    Shortcut {
        sequence: "Ctrl+Shift+W"
        onActivated: window.closeTab(window.current)
    }

    Shortcut {
        sequences: ["Ctrl+Tab", "Ctrl+PgDown"]
        onActivated: window.current = (window.current + 1) % window.views.length
    }

    Shortcut {
        sequences: ["Ctrl+Shift+Tab", "Ctrl+PgUp"]
        onActivated: window.current = (window.current + window.views.length - 1) % window.views.length
    }

    Shortcut {
        sequence: "Ctrl+Shift+C"
        onActivated: window.currentView?.copy()
    }

    Shortcut {
        sequence: "Ctrl+Shift+V"
        onActivated: window.currentView?.paste()
    }

    Shortcut {
        sequences: ["Ctrl++", "Ctrl+="]
        onActivated: window.app.settings.fontSize = Math.min(window.app.settings.fontSize + 1, 32)
    }

    Shortcut {
        sequence: "Ctrl+-"
        onActivated: window.app.settings.fontSize = Math.max(window.app.settings.fontSize - 1, 8)
    }

    Shortcut {
        sequence: "Ctrl+0"
        onActivated: window.app.settings.fontSize = 13
    }

    ColumnLayout {
        id: content

        anchors.fill: parent
        spacing: 0

        // Sekme çubuğu
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: window.tabBarHeight
            color: Theme.color.surfaceSidebar
            // Sürüklenen sekme çubuğun dışına çıkınca da görünsün
            z: 1

            Row {
                id: tabRow

                x: 8
                y: 6
                spacing: 4

                Repeater {
                    id: tabRepeater

                    model: window.views

                    Rectangle {
                        id: tab

                        required property int index
                        required property TerminalView modelData
                        readonly property bool selected: index === window.current
                        // Sürüklenirken bırakılacak yer (tabRow'a göre). Bırakılınca DragHandler
                        // konumu sıfırladığı için sürükleme sırasında saklanır.
                        property point dropPoint

                        width: Math.min(tabLabel.implicitWidth + 54, 220)
                        height: 29
                        radius: Theme.radius.sm
                        color: selected ? window.app.colors.background
                                        : tabHover.hovered ? Theme.color.surfaceHover : "transparent"
                        z: tabDrag.active ? 1 : 0
                        // Çubuğun dışına sürüklenince soluklaşır: bırakılırsa yeni pencere olur
                        opacity: tabDrag.active && (dropPoint.y < -window.detachDistance
                                                    || dropPoint.y > window.tabBarHeight + window.detachDistance)
                                 ? 0.6 : 1
                        transform: Translate {
                            x: tabDrag.active ? tabDrag.activeTranslation.x : 0
                            y: tabDrag.active ? tabDrag.activeTranslation.y : 0
                        }

                        // Alt köşeler düz: sekme altındaki terminale bağlanır
                        Rectangle {
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: parent.radius
                            color: parent.color
                        }

                        Row {
                            x: 14
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 10

                            Label {
                                id: tabLabel

                                width: Math.min(implicitWidth, 160)
                                anchors.verticalCenter: parent.verticalCenter
                                text: tab.modelData ? tab.modelData.title : ""
                                elide: Text.ElideRight
                                color: tab.selected ? Theme.color.textPrimary : Theme.color.textSecondary
                                font.pixelSize: Theme.font.small
                                font.weight: tab.selected ? Font.DemiBold : Font.Normal
                            }

                            HoverButton {
                                anchors.verticalCenter: parent.verticalCenter
                                implicitWidth: 16
                                implicitHeight: 16
                                radius: 8
                                onClicked: window.closeTab(tab.index)

                                Icon {
                                    anchors.centerIn: parent
                                    name: "close"
                                    size: 12
                                    color: Theme.color.textSecondary
                                }
                            }
                        }

                        HoverHandler {
                            id: tabHover
                        }

                        TapHandler {
                            onTapped: window.current = tab.index
                        }

                        DragHandler {
                            id: tabDrag

                            target: null
                            onActiveChanged: {
                                if (active)
                                    window.current = tab.index;
                                else
                                    window.dropTab(tab.index, tab.dropPoint);
                            }
                            onCentroidChanged: {
                                if (active)
                                    tab.dropPoint = tab.mapToItem(tabRow, centroid.position);
                            }
                        }
                    }
                }

                HoverButton {
                    implicitWidth: 28
                    implicitHeight: 28
                    radius: Theme.radius.sm
                    onClicked: window.newTab()

                    Icon {
                        anchors.centerIn: parent
                        name: "plus"
                        size: 14
                        color: Theme.color.textPrimary
                    }
                }
            }
        }

        // Terminaller buraya eklenir (addView), yalnızca seçili olan görünür
        Item {
            id: stack

            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }

    Timer {
        running: window.screenshot.length > 0
        interval: 1500
        onTriggered: content.grabToImage(result => {
            result.saveToFile(window.screenshot);
            Qt.quit();
        })
    }
}
