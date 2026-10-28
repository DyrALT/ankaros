// Terminal penceresi: sekme çubuğu ve sekmeler. Pencere çerçevesini ankaros'ta compositor çizer
// (karar 0007). Figma: "Faz 3 · Uygulamalar" sayfası, Terminal (açık/koyu).
//
// Kısayollar: Ctrl+Shift+T yeni sekme, Ctrl+Shift+W sekmeyi kapat, Ctrl+Tab sonraki sekme,
// Ctrl+Shift+C kopyala, Ctrl+Shift+V yapıştır, Ctrl+artı/eksi/0 yazı boyutu, Shift+PageUp/Down kaydır.
pragma ComponentBehavior: Bound

import QtCore
import QtQuick
import QtQuick.Layouts

import Ankaros.UI

Window {
    id: window

    // Boş değilse açılıştan kısa süre sonra pencerenin görüntüsü bu dosyaya kaydedilir ve uygulama kapanır
    property string screenshot
    property int current: 0
    property int nextId: 1
    readonly property TerminalView currentView: viewAt(window.current)

    // Sekmenin terminali. views.count okunduğu için binding'ler sekme eklenince yeniden hesaplanır.
    function viewAt(index: int): TerminalView {
        return index < views.count ? views.itemAt(index) as TerminalView : null;
    }

    function newTab() {
        tabs.append({ uid: window.nextId++ });
        window.current = tabs.count - 1;
    }

    function closeTab(index: int) {
        tabs.remove(index);
        if (tabs.count === 0) {
            Qt.quit();
            return;
        }
        window.current = Math.min(index, tabs.count - 1);
    }

    width: 880
    height: 560
    minimumWidth: 420
    minimumHeight: 240
    visible: true
    title: qsTr("Terminal")
    color: colors.background

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

    Settings {
        id: settings

        category: "terminal"
        property int fontSize: 13
    }

    ListModel {
        id: tabs

        Component.onCompleted: window.newTab()
    }

    Shortcut {
        sequence: "Ctrl+Shift+T"
        onActivated: window.newTab()
    }

    Shortcut {
        sequence: "Ctrl+Shift+W"
        onActivated: window.closeTab(window.current)
    }

    Shortcut {
        sequences: ["Ctrl+Tab", "Ctrl+PgDown"]
        onActivated: window.current = (window.current + 1) % tabs.count
    }

    Shortcut {
        sequences: ["Ctrl+Shift+Tab", "Ctrl+PgUp"]
        onActivated: window.current = (window.current + tabs.count - 1) % tabs.count
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
        onActivated: settings.fontSize = Math.min(settings.fontSize + 1, 32)
    }

    Shortcut {
        sequence: "Ctrl+-"
        onActivated: settings.fontSize = Math.max(settings.fontSize - 1, 8)
    }

    Shortcut {
        sequence: "Ctrl+0"
        onActivated: settings.fontSize = 13
    }

    ColumnLayout {
        id: content

        anchors.fill: parent
        spacing: 0

        // Sekme çubuğu
        Rectangle {
            Layout.fillWidth: true
            implicitHeight: 35
            color: Theme.color.surfaceSidebar

            Row {
                x: 8
                y: 6
                spacing: 4

                Repeater {
                    model: tabs

                    Rectangle {
                        id: tab

                        required property int index
                        readonly property bool selected: index === window.current
                        readonly property TerminalView view: window.viewAt(index)

                        width: Math.min(tabRow.implicitWidth + 28, 220)
                        height: 29
                        radius: Theme.radius.sm
                        color: selected ? window.colors.background
                                        : tabHover.hovered ? Theme.color.surfaceHover : "transparent"

                        // Alt köşeler düz: sekme altındaki terminale bağlanır
                        Rectangle {
                            anchors.bottom: parent.bottom
                            width: parent.width
                            height: parent.radius
                            color: parent.color
                        }

                        Row {
                            id: tabRow

                            x: 14
                            anchors.verticalCenter: parent.verticalCenter
                            spacing: 10

                            Label {
                                width: Math.min(implicitWidth, 160)
                                anchors.verticalCenter: parent.verticalCenter
                                text: tab.view ? tab.view.title : ""
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

        StackLayout {
            id: stack

            Layout.fillWidth: true
            Layout.fillHeight: true
            currentIndex: window.current

            Repeater {
                id: views

                model: tabs

                TerminalView {
                    id: view

                    required property int index

                    fontSize: settings.fontSize
                    foreground: window.colors.foreground
                    background: window.colors.background
                    cursorColor: Theme.color.accent
                    selectionColor: Theme.color.accentSoft
                    ansiColors: window.colors.ansi
                    focus: index === window.current
                    onFinished: window.closeTab(index)
                    Component.onCompleted: if (focus) forceActiveFocus()
                }
            }
        }
    }

    onCurrentChanged: Qt.callLater(() => window.currentView?.forceActiveFocus())

    Timer {
        running: window.screenshot.length > 0
        interval: 1500
        onTriggered: content.grabToImage(result => {
            result.saveToFile(window.screenshot);
            Qt.quit();
        })
    }
}
