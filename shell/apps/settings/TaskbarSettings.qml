// Ayarlar › Alt çubuk: sabitlenmiş uygulamalar ve saat. Uygulamalar anahtarla sabitlenir, tutamaktan
// sürüklenerek sıralanır. Seçimler Appearance'a yazılır, alt çubuk hemen değişir.
// Figma: "Faz 3 · Uygulamalar" sayfası.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Ankaros.System
import Ankaros.UI

SettingsPage {
    id: root

    readonly property int rowHeight: 40

    title: qsTr("Alt çubuk")

    // Sabitlenmiş uygulamalar sırasıyla önde, diğerleri listedeki sırasıyla arkada
    function fill() {
        const pinned = Appearance.customPinnedApps ? Appearance.pinnedApps : AppCatalog.pinned;
        const names = pinned.filter(name => AppCatalog.names.includes(name))
            .concat(AppCatalog.names.filter(name => !pinned.includes(name)));
        apps.clear();
        for (const name of names)
            apps.append({ name: name, pinned: pinned.includes(name) });
    }

    function save() {
        const pinned = [];
        for (let i = 0; i < apps.count; ++i) {
            if (apps.get(i).pinned)
                pinned.push(apps.get(i).name);
        }
        Appearance.pinnedApps = pinned;
    }

    Component.onCompleted: fill()

    ListModel {
        id: apps
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 6

        SettingLabel {
            text: qsTr("Sabitlenmiş uygulamalar")
            info: qsTr("Açık olanlar alt çubukta her zaman görünür. Sırayı değiştirmek için soldaki tutamaktan sürükleyin.")
        }

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: Math.min(apps.count, 6) * root.rowHeight + 8
            radius: 14
            color: Theme.color.surfaceHover

            ListView {
                id: list

                anchors.fill: parent
                anchors.topMargin: 4
                anchors.bottomMargin: 4
                clip: true
                model: apps
                boundsBehavior: Flickable.StopAtBounds

                delegate: Item {
                    id: row

                    required property int index
                    required property string name
                    required property bool pinned
                    // Sürüklenirken satırın kaç kez yer değiştirdiği. Satır komşusunun yarısını
                    // geçince yer değiştirir, tutamağın kayması da bir satır azalır.
                    property int moved: 0
                    property bool dragging: false

                    width: ListView.view.width
                    height: root.rowHeight
                    z: row.dragging ? 1 : 0

                    Rectangle {
                        id: content

                        width: parent.width
                        height: parent.height
                        radius: Theme.radius.sm
                        color: row.dragging ? Theme.color.surfaceWindow : "transparent"
                        border.color: row.dragging ? Theme.color.border : "transparent"

                        RowLayout {
                            anchors.fill: parent
                            anchors.leftMargin: 12
                            anchors.rightMargin: 12
                            spacing: 12

                            Label {
                                text: "⋮⋮"
                                color: Theme.color.textSecondary

                                // MouseArea, çünkü liste kaydırması DragHandler'ın sürüklemesini
                                // elinden alıyor. preventStealing bunu engeller.
                                MouseArea {
                                    property real startY

                                    anchors.fill: parent
                                    anchors.margins: -8
                                    cursorShape: Qt.SizeVerCursor
                                    preventStealing: true
                                    onPressed: mouse => {
                                        startY = mapToItem(list, mouse.x, mouse.y).y;
                                        row.moved = 0;
                                        row.dragging = true;
                                    }
                                    onPositionChanged: mouse => {
                                        let offset = mapToItem(list, mouse.x, mouse.y).y - startY
                                            - row.moved * root.rowHeight;
                                        if (offset > root.rowHeight / 2 && row.index < apps.count - 1) {
                                            apps.move(row.index, row.index + 1, 1);
                                            row.moved += 1;
                                            offset -= root.rowHeight;
                                        } else if (offset < -root.rowHeight / 2 && row.index > 0) {
                                            apps.move(row.index, row.index - 1, 1);
                                            row.moved -= 1;
                                            offset += root.rowHeight;
                                        }
                                        content.y = offset;
                                    }
                                    onReleased: {
                                        row.dragging = false;
                                        content.y = 0;
                                        root.save();
                                    }
                                    onCanceled: {
                                        row.dragging = false;
                                        content.y = 0;
                                        root.save();
                                    }
                                }
                            }

                            AppIcon {
                                app: row.name
                                size: 28
                            }

                            Label {
                                Layout.fillWidth: true
                                text: row.name
                                font.weight: Font.Medium
                            }

                            ToggleSwitch {
                                checked: row.pinned
                                onToggled: checked => {
                                    apps.setProperty(row.index, "pinned", checked);
                                    root.save();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 12

        SectionTitle {
            text: qsTr("Saat")
        }

        RowLayout {
            Layout.fillWidth: true

            SettingLabel {
                Layout.fillWidth: true
                text: qsTr("Saat biçimi")
            }

            SegmentedControl {
                options: [{ value: "24", label: qsTr("24 saat") }, { value: "12", label: qsTr("12 saat") }]
                value: Appearance.clockFormat
                onSelected: value => Appearance.clockFormat = value
            }
        }

        RowLayout {
            Layout.fillWidth: true

            SettingLabel {
                Layout.fillWidth: true
                text: qsTr("Tarihi göster")
            }

            ToggleSwitch {
                checked: Appearance.showDate
                onToggled: checked => Appearance.showDate = checked
            }
        }
    }
}
