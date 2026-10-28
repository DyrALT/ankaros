// Çizgi simge. Yol verisi IconData.js'ten gelir ve Qt Shapes ile GPU'da çizilir, bu yüzden
// her boyutta keskin görünür ve rengi temaya göre değişebilir.
//
// Çizici: Qt'nin klasik GeometryRenderer'ı (çizgiyi üçgenlere böler). Kenarlar süper örneklemeyle
// yumuşatılır: simge 4 kat büyük bir dokuya çizilir, mipmap'li doğrusal süzgeçle küçültülür. Bu, GPU'nun
// MSAA desteğine bağlı değildir (virgl'de layer.samples etkisizdi, kenarlar tırtıklı kalıyordu).
// CurveRenderer kullanılmaz: çizgiyi piksel shader'ında eğriye uzaklıkla hesaplar ve virgl/ANGLE (UTM,
// Mac'in GPU'su) üzerinde ince çizgileri kesik kesik çiziyordu.
import QtQuick
import QtQuick.Shapes
import QtQuick.Window

import "IconData.js" as IconData

Item {
    id: root

    property string name
    property color color: Theme.color.textPrimary
    property real size: 24

    implicitWidth: size
    implicitHeight: size
    layer.enabled: true
    layer.textureSize: Qt.size(Math.ceil(width * supersampling), Math.ceil(height * supersampling))
    layer.smooth: true
    layer.mipmap: true

    // Ekranın piksel yoğunluğu da hesaba katılır (Retina ekranda önizleme)
    readonly property real supersampling: 4 * Screen.devicePixelRatio

    Shape {
        // Yollar 24x24'lük alana göre yazıldı. Ölçekleme çizgi kalınlığını da orantılı büyütür,
        // Figma'daki simgelerle aynı görünüm elde edilir.
        width: 24
        height: 24
        anchors.centerIn: parent
        scale: root.size / 24

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
