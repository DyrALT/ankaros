// Buzlu cam yüzey: arkasındaki öğenin (genellikle duvar kağıdı) ilgili bölümünü bulanıklaştırır,
// üzerine yarı saydam tema rengi ve ince bir kenarlık koyar. İçerik çocuk olarak verilir.
// backdrop verilmezse sadece yarı saydam renk çizilir.
import QtQuick
import QtQuick.Effects

Item {
    id: root

    property Item backdrop
    property real radius: Theme.radius.lg
    property color tint: Theme.color.surfacePanel
    property color borderColor: Theme.color.border
    property bool shadow: true
    default property alias content: contentItem.data

    RectangularShadow {
        anchors.fill: parent
        visible: root.shadow
        radius: root.radius
        offset.y: 8
        blur: 32
        color: Qt.rgba(0, 0, 0, 0.12)
    }

    ShaderEffectSource {
        id: backdropSource

        anchors.fill: parent
        visible: false
        sourceItem: root.backdrop
        sourceRect: root.backdrop ? root.backdrop.mapFromItem(root, 0, 0, root.width, root.height)
                                  : Qt.rect(0, 0, 0, 0)
    }

    MultiEffect {
        anchors.fill: parent
        visible: root.backdrop !== null
        source: backdropSource
        autoPaddingEnabled: false
        blurEnabled: true
        blur: 1
        blurMax: 48
        maskEnabled: true
        maskSource: mask
    }

    Item {
        id: mask

        anchors.fill: parent
        visible: false
        layer.enabled: true

        Rectangle {
            anchors.fill: parent
            radius: root.radius
        }
    }

    Rectangle {
        anchors.fill: parent
        radius: root.radius
        color: root.tint
        border.color: root.borderColor
        border.width: 1
    }

    Item {
        id: contentItem

        anchors.fill: parent
    }
}
