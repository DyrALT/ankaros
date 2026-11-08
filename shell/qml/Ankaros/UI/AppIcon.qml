// Uygulama simgesi. Kendi çizdiğimiz uygulamalar için degrade bir kare üzerinde beyaz (açık renkli
// simgelerde koyu) çizgi simge: renkler ve simge IconData.apps tablosundan uygulama adına göre seçilir.
// Tabloda olmayan ama kendi resmi olan (ank kur ile kurulan) uygulamanın logosu olduğu gibi,
// kutucuğa sokulmadan gösterilir. İkisi de yoksa genel simge.
import QtQuick
import QtQuick.Window

import "IconData.js" as IconData

Item {
    id: root

    property string app
    property real size: 64

    readonly property var ownStyle: IconData.apps[app]
    readonly property string image: ownStyle ? "" : (AppImages.map[app] ?? "")
    // Resim bozuksa (Image.Error) genel simgeye dönülür
    readonly property bool showImage: image !== "" && logo.status !== Image.Error
    readonly property var style: ownStyle ?? IconData.fallbackApp

    implicitWidth: size
    implicitHeight: size

    Rectangle {
        anchors.fill: parent
        visible: !root.showImage
        radius: root.size * 0.234
        border.color: Qt.rgba(1, 1, 1, 0.18)
        border.width: 1

        gradient: Gradient {
            GradientStop {
                position: 0
                color: root.style.top
            }
            GradientStop {
                position: 1
                color: root.style.bottom
            }
        }

        Icon {
            anchors.centerIn: parent
            name: root.style.icon
            size: root.size * 0.5
            color: root.style.darkGlyph ? "#6b4a00" : "#ffffff"
        }
    }

    Image {
        id: logo

        anchors.fill: parent
        visible: root.showImage
        source: root.image
        fillMode: Image.PreserveAspectFit
        asynchronous: true
        smooth: true
        mipmap: true
        // SVG ve büyük PNG'ler simge boyutunda (ekran ölçeğine göre) çözülür, keskin kalır
        sourceSize: Qt.size(root.size * Screen.devicePixelRatio, root.size * Screen.devicePixelRatio)
    }
}
