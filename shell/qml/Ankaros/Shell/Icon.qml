// Çizgi simge. Yol verisi IconData.js'ten gelir ve Qt Shapes ile GPU'da çizilir, bu yüzden
// her boyutta keskin görünür ve rengi temaya göre değişebilir.
import QtQuick
import QtQuick.Shapes

import "IconData.js" as IconData

Item {
    id: root

    property string name
    property color color: Theme.color.textPrimary
    property real size: 24

    implicitWidth: size
    implicitHeight: size

    Shape {
        // Yollar 24x24'lük alana göre yazıldı. Ölçekleme çizgi kalınlığını da orantılı büyütür,
        // Figma'daki simgelerle aynı görünüm elde edilir.
        width: 24
        height: 24
        anchors.centerIn: parent
        scale: root.size / 24
        preferredRendererType: Shape.CurveRenderer

        ShapePath {
            strokeColor: root.color
            strokeWidth: 2
            fillColor: "transparent"
            capStyle: ShapePath.RoundCap
            joinStyle: ShapePath.RoundJoin

            PathSvg {
                path: IconData.paths[root.name] ?? ""
            }
        }
    }
}
