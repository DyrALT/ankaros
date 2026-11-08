// Terminal uygulaması: pencereler (TerminalWindow) ve sekmelerin terminalleri (TerminalView). Bir süreç
// birden çok pencere açabilir. Terminaller pencereye değil buraya aittir: sekme bir pencereden ötekine
// taşınınca içinde çalışan kabuk kapanmaz. Son pencere kapanınca uygulama çıkar.
pragma ComponentBehavior: Bound

import QtCore
import QtQuick

import Ankaros.UI

QtObject {
    id: app

    // Boş değilse ilk pencerenin görüntüsü açılıştan kısa süre sonra bu dosyaya kaydedilir (CI)
    property string screenshot
    property list<TerminalWindow> windows

    readonly property TerminalSettings settings: TerminalSettings {}

    component TerminalSettings: Settings {
        category: "terminal"
        property int fontSize: 13
    }

    // Renkler temaya göre. 0-7 normal, 8-15 parlak ANSI renkleri.
    readonly property Colors colors: Colors {}

    component Colors: QtObject {
        readonly property color background: Theme.color.surfaceWindow
        readonly property color foreground: Theme.color.textPrimary
        readonly property list<color> ansi: Theme.dark
            ? ["#2c2927", "#e5604a", "#7fc28f", "#e8b85c", "#7fa8e0", "#d39bd0", "#6fc4c4", "#d6d1cb",
               "#6b6560", "#f07b66", "#9bd6a9", "#f2cc7e", "#9dbfee", "#e3b4e0", "#8fd8d8", "#f5f2ee"]
            : ["#1d1b19", "#c0392b", "#3d8b4f", "#a8741a", "#2f6fb3", "#9b4f96", "#2b8a8a", "#b8b2ab",
               "#6b6560", "#d4523f", "#4fa865", "#c99a2e", "#3f7fc6", "#b86bb3", "#3aa5a5", "#8a847e"]
    }

    readonly property Component windowComponent: Component {
        TerminalWindow {}
    }

    readonly property Component viewComponent: Component {
        TerminalView {
            id: view

            anchors.fill: parent
            fontSize: app.settings.fontSize
            foreground: app.colors.foreground
            background: app.colors.background
            cursorColor: Theme.color.accent
            selectionColor: Theme.color.accentSoft
            ansiColors: app.colors.ansi
            onFinished: app.viewFinished(view)
        }
    }

    // Yeni bir terminal (kabuğuyla birlikte). parent: ilk boyutunu alacağı yer.
    function createView(parent: Item): TerminalView {
        return viewComponent.createObject(parent) as TerminalView;
    }

    // Yeni pencere: view verilirse o sekmeyle (başka pencereden taşınan), verilmezse yeni bir sekmeyle
    function newWindow(view: TerminalView, size: size): TerminalWindow {
        const properties = { app: app, screenshot: windows.length === 0 ? screenshot : "" };
        if (size.width > 0) {
            properties.width = size.width;
            properties.height = size.height;
        }
        const window = windowComponent.createObject(app, properties) as TerminalWindow;
        windows = Array.from(windows).concat([window]);
        if (view)
            window.addView(view, -1);
        else
            window.newTab();
        window.visible = true;
        return window;
    }

    function windowClosed(window: TerminalWindow) {
        if (!windows.includes(window))
            return;
        windows = Array.from(windows).filter(w => w !== window);
        window.visible = false;
        window.destroy();
        if (windows.length === 0)
            Qt.quit();
    }

    // Sekmedeki kabuk çıktı (exit): sekmeyi hangi penceredeyse orada kapat
    function viewFinished(view: TerminalView) {
        for (const window of windows) {
            const index = window.views.indexOf(view);
            if (index >= 0) {
                window.closeTab(index);
                return;
            }
        }
    }

    Component.onCompleted: newWindow(null, Qt.size(0, 0))
}
