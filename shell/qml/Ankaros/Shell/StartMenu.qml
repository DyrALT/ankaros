// Başlat paneli: arama, sabitlenmiş uygulamalar, son kullanılanlar ve kullanıcı satırı. "Tümü" bütün
// uygulamaları alfabetik listeler. Arama uygulamaları ve Ayarlar'daki ayarları bulur (SettingsIndex.js).
// Alt çubuğun hemen üstünde, ortada açılır. Esc ya da panel dışına tıklama kapatır.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Ankaros.UI

import "SettingsIndex.js" as SettingsIndex

GlassPanel {
    id: root

    property bool open: false
    property var apps: []
    // Son kullanılan uygulamalar, en yeniden eskiye: [{ app: "Terminal", time: Date }, ...]
    property var recentApps: []
    property string userName
    // "home" (sabitlenmiş ve son kullanılanlar) ya da "all" (bütün uygulamalar)
    property string view: "home"
    // "… dakika önce" yazılarının hesaplandığı an, panel her açıldığında güncellenir
    property date now: new Date()

    readonly property string query: search.text.trim().toLocaleLowerCase()
    // Arama sonuçları: önce uygulamalar, sonra ayarlar
    readonly property var results: query.length === 0 ? [] : apps
        .filter(app => app.toLocaleLowerCase().includes(query))
        .map(app => ({ kind: "app", app: app, title: app, subtitle: qsTr("Uygulama"), page: "" }))
        .concat(SettingsIndex.search(query).map(entry => ({
            kind: "setting",
            app: "Ayarlar",
            title: entry.title,
            subtitle: qsTr("Ayarlar › %1").arg(entry.pageTitle),
            page: entry.page
        })))
    // Enter'ın açacağı sonuç, oklarla değişir
    property int currentResult: 0
    // Bütün uygulamalar baş harflerine göre: [{ letter: "A", apps: ["Ayarlar"] }, ...]
    readonly property var groups: {
        const groups = [];
        for (const app of apps.slice().sort((a, b) => a.localeCompare(b, "tr"))) {
            const letter = app.charAt(0).toLocaleUpperCase();
            if (groups.length === 0 || groups[groups.length - 1].letter !== letter)
                groups.push({ letter: letter, apps: [] });
            groups[groups.length - 1].apps.push(app);
        }
        return groups;
    }

    signal appLaunched(string app)
    // Ayarlar'ın bir sayfası istendi (aramada bir ayar seçildi)
    signal settingsRequested(string page)
    signal lockRequested
    signal logoutRequested
    signal rebootRequested
    signal poweroffRequested

    function focusSearch() {
        search.forceActiveFocus();
    }

    function setQuery(text) {
        search.text = text;
    }

    function activateCurrent() {
        if (results.length > 0)
            activate(results[currentResult]);
    }

    function activate(result) {
        if (result.kind === "setting")
            settingsRequested(result.page);
        else
            appLaunched(result.app);
    }

    // "az önce", "5 dakika önce", "Dün"...
    function ago(time) {
        const minutes = Math.floor((now.getTime() - time.getTime()) / 60000);
        if (minutes < 1)
            return qsTr("az önce");
        if (minutes < 60)
            return qsTr("%1 dakika önce").arg(minutes);
        if (minutes < 24 * 60)
            return qsTr("%1 saat önce").arg(Math.floor(minutes / 60));
        const days = Math.floor(minutes / (24 * 60));
        return days === 1 ? qsTr("Dün") : qsTr("%1 gün önce").arg(days);
    }

    implicitWidth: Theme.size.startMenuWidth
    implicitHeight: column.implicitHeight + 44
    radius: 24
    visible: opacity > 0
    opacity: open ? 1 : 0
    transform: Translate {
        y: root.open ? 0 : 16

        Behavior on y {
            NumberAnimation { duration: Theme.motion.normal; easing.type: Theme.motion.easing }
        }
    }

    Behavior on opacity {
        NumberAnimation { duration: Theme.motion.normal; easing.type: Theme.motion.easing }
    }

    onQueryChanged: currentResult = 0
    onOpenChanged: {
        search.text = "";
        view = "home";
        now = new Date();
        powerMenu.opacity = 0;
        if (open)
            search.forceActiveFocus();
    }

    Keys.onEscapePressed: root.open = false

    // Panelin içindeki tıklamalar arkadaki "dışarı tıklayınca kapat" alanına ulaşmasın.
    MouseArea {
        anchors.fill: parent
    }

    component SectionHeader: RowLayout {
        id: header

        property string title
        // Sağdaki düğmenin yazısı, boşsa düğme yok
        property string action

        signal actionClicked

        Layout.fillWidth: true
        Layout.preferredHeight: 24

        Label {
            Layout.fillWidth: true
            text: header.title
            font.weight: Font.DemiBold
        }

        HoverButton {
            visible: header.action.length > 0
            Layout.preferredWidth: actionLabel.implicitWidth + 20
            Layout.preferredHeight: 24
            radius: Theme.radius.sm
            restColor: Theme.color.surfaceHover
            onClicked: header.actionClicked()

            Label {
                id: actionLabel

                anchors.centerIn: parent
                text: header.action
                font.pixelSize: Theme.font.small
                font.weight: Font.Medium
            }
        }
    }

    ColumnLayout {
        id: column

        anchors.fill: parent
        anchors.margins: Theme.spacing.xl
        anchors.bottomMargin: 20
        spacing: 20

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 44
            radius: Theme.radius.md
            color: Theme.color.surfaceHover
            border.color: Theme.color.accent
            border.width: 1.5

            Icon {
                id: searchIcon

                anchors.verticalCenter: parent.verticalCenter
                x: 14
                name: "search"
                size: 16
                color: Theme.color.textSecondary
            }

            TextInput {
                id: search

                anchors.left: searchIcon.right
                anchors.leftMargin: 10
                anchors.right: parent.right
                anchors.rightMargin: 14
                anchors.verticalCenter: parent.verticalCenter
                color: Theme.color.textPrimary
                selectionColor: Theme.color.accent
                font.family: Theme.font.family
                font.pixelSize: Theme.font.large
                clip: true
                Keys.onEscapePressed: root.open = false
                Keys.onReturnPressed: root.activateCurrent()
                Keys.onEnterPressed: root.activateCurrent()
                Keys.onDownPressed: root.currentResult = Math.min(root.currentResult + 1, root.results.length - 1)
                Keys.onUpPressed: root.currentResult = Math.max(root.currentResult - 1, 0)

                Label {
                    anchors.verticalCenter: parent.verticalCenter
                    visible: search.text.length === 0
                    text: qsTr("Uygulama, dosya ya da ayar ara")
                    color: Theme.color.textSecondary
                    font.pixelSize: Theme.font.large
                }
            }
        }

        // Ana görünüm: sabitlenmiş ve son kullanılanlar
        SectionHeader {
            visible: root.query.length === 0 && root.view === "home"
            title: qsTr("Sabitlenmiş")
            action: qsTr("Tümü  ›")
            onActionClicked: root.view = "all"
        }

        Grid {
            Layout.fillWidth: true
            visible: root.query.length === 0 && root.view === "home"
            columns: 6
            rowSpacing: Theme.spacing.md

            Repeater {
                model: root.apps.slice(0, 12)

                AppTile {
                    required property string modelData

                    app: modelData
                    onClicked: root.appLaunched(modelData)
                }
            }
        }

        SectionHeader {
            visible: root.query.length === 0 && root.view === "home" && root.recentApps.length > 0
            title: qsTr("Son kullanılanlar")
        }

        GridLayout {
            Layout.fillWidth: true
            visible: root.query.length === 0 && root.view === "home" && root.recentApps.length > 0
            columns: 2
            columnSpacing: Theme.spacing.sm
            rowSpacing: Theme.spacing.xs

            Repeater {
                model: root.recentApps.slice(0, 4)

                RecommendedItem {
                    required property var modelData

                    Layout.fillWidth: true
                    app: modelData.app
                    title: modelData.app
                    subtitle: root.ago(modelData.time)
                    onClicked: root.appLaunched(modelData.app)
                }
            }
        }

        // Bütün uygulamalar, baş harflerine göre iki sütunda
        SectionHeader {
            visible: root.query.length === 0 && root.view === "all"
            title: qsTr("Tüm uygulamalar")
            action: qsTr("‹  Geri")
            onActionClicked: root.view = "home"
        }

        Flickable {
            id: allApps

            // Grupları iki sütuna böl: ilk sütun öğelerin (başlıklar dahil) yarısını alır
            readonly property int splitAt: {
                const sizes = root.groups.map(group => group.apps.length + 1);
                const total = sizes.reduce((sum, size) => sum + size, 0);
                let count = 0;
                for (let i = 0; i < sizes.length; ++i) {
                    if (count >= total / 2)
                        return i;
                    count += sizes[i];
                }
                return sizes.length;
            }

            Layout.fillWidth: true
            Layout.preferredHeight: Math.min(contentHeight, 380)
            visible: root.query.length === 0 && root.view === "all"
            contentHeight: columns.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            Row {
                id: columns

                width: allApps.width
                spacing: Theme.spacing.xl

                Repeater {
                    model: [root.groups.slice(0, allApps.splitAt), root.groups.slice(allApps.splitAt)]

                    Column {
                        id: groupColumn

                        required property var modelData

                        width: (columns.width - columns.spacing) / 2
                        spacing: 2

                        Repeater {
                            model: groupColumn.modelData

                            Column {
                                id: group

                                required property var modelData

                                width: groupColumn.width
                                spacing: 2

                                Label {
                                    leftPadding: Theme.spacing.sm
                                    topPadding: Theme.spacing.sm
                                    bottomPadding: 2
                                    text: group.modelData.letter
                                    color: Theme.color.textSecondary
                                    font.pixelSize: Theme.font.caption
                                    font.weight: Font.DemiBold
                                }

                                Repeater {
                                    model: group.modelData.apps

                                    HoverButton {
                                        id: appRow

                                        required property string modelData

                                        width: group.width
                                        height: 40
                                        radius: 10
                                        onClicked: root.appLaunched(modelData)

                                        Row {
                                            anchors.verticalCenter: parent.verticalCenter
                                            x: Theme.spacing.sm
                                            spacing: Theme.spacing.md

                                            AppIcon {
                                                anchors.verticalCenter: parent.verticalCenter
                                                app: appRow.modelData
                                                size: 28
                                            }

                                            Label {
                                                anchors.verticalCenter: parent.verticalCenter
                                                text: appRow.modelData
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        // Arama sonuçları
        Column {
            Layout.fillWidth: true
            visible: root.query.length > 0
            spacing: 2

            Repeater {
                model: root.results

                Column {
                    id: result

                    required property var modelData
                    required property int index
                    // Bir grubun ilk sonucunun üstünde grubun adı yazar
                    readonly property bool firstOfKind: index === 0
                                                        || root.results[index - 1].kind !== modelData.kind

                    width: parent.width
                    spacing: 2

                    Label {
                        visible: result.firstOfKind
                        leftPadding: Theme.spacing.sm
                        topPadding: result.index === 0 ? 0 : Theme.spacing.sm
                        bottomPadding: 2
                        text: result.modelData.kind === "app" ? qsTr("Uygulamalar") : qsTr("Ayarlar")
                        color: Theme.color.textSecondary
                        font.pixelSize: Theme.font.caption
                        font.weight: Font.DemiBold
                    }

                    HoverButton {
                        width: parent.width
                        height: 50
                        radius: 10
                        checked: result.index === root.currentResult
                        onClicked: root.activate(result.modelData)

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: Theme.spacing.sm
                            anchors.rightMargin: Theme.spacing.md
                            spacing: Theme.spacing.md

                            AppIcon {
                                app: result.modelData.app
                                size: 32
                            }

                            Column {
                                Layout.fillWidth: true
                                spacing: 2

                                Label {
                                    text: result.modelData.title
                                    font.weight: Font.DemiBold
                                }

                                Label {
                                    text: result.modelData.subtitle
                                    color: Theme.color.textSecondary
                                    font.pixelSize: Theme.font.caption
                                }
                            }

                            // Enter'ın açacağı sonuç
                            Rectangle {
                                visible: result.index === root.currentResult
                                Layout.preferredWidth: enterLabel.implicitWidth + 16
                                Layout.preferredHeight: 22
                                radius: 6
                                color: "transparent"
                                border.color: Theme.color.border

                                Label {
                                    id: enterLabel

                                    anchors.centerIn: parent
                                    text: "Enter"
                                    color: Theme.color.textSecondary
                                    font.pixelSize: Theme.font.caption
                                }
                            }
                        }
                    }
                }
            }
        }

        Label {
            visible: root.query.length > 0 && root.results.length === 0
            text: qsTr("“%1” ile eşleşen uygulama ya da ayar yok").arg(search.text)
            color: Theme.color.textSecondary
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.color.border
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 10

            Avatar {
                name: root.userName
                size: 32
            }

            Label {
                Layout.fillWidth: true
                text: root.userName
                font.weight: Font.Medium
            }

            HoverButton {
                Layout.preferredWidth: 36
                Layout.preferredHeight: 36
                radius: Theme.radius.sm
                onClicked: root.appLaunched("Ayarlar")

                AppIcon {
                    anchors.centerIn: parent
                    app: "Ayarlar"
                    size: 28
                }
            }

            HoverButton {
                id: powerButton

                Layout.preferredWidth: 36
                Layout.preferredHeight: 36
                radius: Theme.radius.sm
                checked: powerMenu.opacity > 0
                onClicked: powerMenu.opacity = powerMenu.opacity > 0 ? 0 : 1

                Icon {
                    anchors.centerIn: parent
                    name: "power"
                    size: 18
                }
            }
        }
    }

    PowerMenu {
        id: powerMenu

        x: parent.width - width - 22
        y: parent.height - height - 64
        opacity: 0
        onLockRequested: {
            root.open = false;
            root.lockRequested();
        }
        onLogoutRequested: root.logoutRequested()
        onRebootRequested: root.rebootRequested()
        onPoweroffRequested: root.poweroffRequested()
    }
}
