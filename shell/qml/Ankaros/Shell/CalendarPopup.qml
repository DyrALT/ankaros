// Alt çubuktaki saate tıklanınca açılan takvim: bugünün tarihi ve ay görünümü (Figma: Takvim).
// Hafta pazartesi başlar, bugün vurgu renginde. Oklar ayı değiştirir, panel her açıldığında bugüne döner.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts

import Ankaros.UI

GlassPanel {
    id: root

    property bool open: false
    property date today: new Date()
    // Gösterilen ay (ayın 1'i)
    property date month: new Date(today.getFullYear(), today.getMonth(), 1)

    // Ay görünümündeki 42 gün (6 hafta): önceki ve sonraki aydan taşan günler dahil
    readonly property var days: {
        const first = new Date(month.getFullYear(), month.getMonth(), 1);
        const offset = (first.getDay() + 6) % 7; // pazartesi 0
        const result = [];
        for (let i = 0; i < 42; ++i)
            result.push(new Date(month.getFullYear(), month.getMonth(), 1 - offset + i));
        // Son hafta tamamen sonraki aydaysa gösterme
        return result[35].getMonth() !== month.getMonth() ? result.slice(0, 35) : result;
    }

    function shiftMonth(delta) {
        month = new Date(month.getFullYear(), month.getMonth() + delta, 1);
    }

    implicitWidth: column.implicitWidth + 2 * Theme.spacing.lg
    implicitHeight: column.implicitHeight + 2 * Theme.spacing.lg
    radius: Theme.radius.lg
    visible: opacity > 0
    opacity: open ? 1 : 0
    transform: Translate {
        y: root.open ? 0 : 12

        Behavior on y {
            NumberAnimation { duration: Theme.motion.normal; easing.type: Theme.motion.easing }
        }
    }

    Behavior on opacity {
        NumberAnimation { duration: Theme.motion.normal; easing.type: Theme.motion.easing }
    }

    onOpenChanged: {
        if (open) {
            today = new Date();
            month = new Date(today.getFullYear(), today.getMonth(), 1);
        }
    }

    // Paneldeki tıklamalar arkadaki "dışarı tıklayınca kapat" alanına ulaşmasın
    MouseArea {
        anchors.fill: parent
    }

    ColumnLayout {
        id: column

        x: Theme.spacing.lg
        y: Theme.spacing.lg
        spacing: Theme.spacing.md

        Column {
            Layout.leftMargin: 6
            spacing: 2

            Label {
                text: root.today.toLocaleDateString(Qt.locale(), "dddd")
                color: Theme.color.textSecondary
                font.pixelSize: Theme.font.small
            }

            Label {
                text: root.today.toLocaleDateString(Qt.locale(), "d MMMM yyyy")
                font.pixelSize: 18
                font.weight: Font.DemiBold
            }
        }

        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 1
            color: Theme.color.border
        }

        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: 6

            Label {
                Layout.fillWidth: true
                text: root.month.toLocaleDateString(Qt.locale(), "MMMM yyyy")
                font.weight: Font.DemiBold
            }

            Repeater {
                model: [{ icon: "chevronLeft", delta: -1 }, { icon: "chevronRight", delta: 1 }]

                HoverButton {
                    id: arrow

                    required property var modelData

                    Layout.preferredWidth: 28
                    Layout.preferredHeight: 28
                    radius: Theme.radius.sm
                    onClicked: root.shiftMonth(modelData.delta)

                    Icon {
                        anchors.centerIn: parent
                        name: arrow.modelData.icon
                        size: 14
                    }
                }
            }
        }

        Grid {
            columns: 7
            rowSpacing: 2

            // Haftanın günleri, pazartesiden başlayarak (Türkçe iki harfli kısaltmalar)
            Repeater {
                model: [qsTr("Pt"), qsTr("Sa"), qsTr("Ça"), qsTr("Pe"), qsTr("Cu"), qsTr("Ct"), qsTr("Pz")]

                Label {
                    required property string modelData

                    width: 40
                    height: 28
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    text: modelData
                    color: Theme.color.textSecondary
                    font.pixelSize: Theme.font.caption
                    font.weight: Font.DemiBold
                }
            }

            Repeater {
                model: root.days

                Item {
                    id: day

                    required property date modelData
                    readonly property bool isToday: modelData.toDateString() === root.today.toDateString()
                    readonly property bool inMonth: modelData.getMonth() === root.month.getMonth()

                    width: 40
                    height: 34

                    Rectangle {
                        anchors.centerIn: parent
                        width: 30
                        height: 30
                        radius: 15
                        visible: day.isToday
                        color: Theme.color.accent
                    }

                    Label {
                        anchors.centerIn: parent
                        text: day.modelData.getDate()
                        color: day.isToday ? Theme.color.textOnAccent
                             : day.inMonth ? Theme.color.textPrimary : Theme.color.textSecondary
                        font.pixelSize: Theme.font.small
                        font.weight: day.isToday ? Font.DemiBold : Font.Normal
                    }
                }
            }
        }
    }
}
