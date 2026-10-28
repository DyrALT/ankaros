// Varsayılan duvar kağıdı: dikey bir renk geçişi üzerinde yumuşak renk lekeleri.
// Lekeler Figma'daki bulanıklaştırılmış elipslerin karşılığıdır. Bulanıklık efekti yerine radyal
// degrade kullanıldığı için çizimi ucuzdur. Konumlar 1440x900'lük tasarıma göre verilip ekrana
// ölçeklenir.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes

import Ankaros.UI

Rectangle {
    id: root

    // [renk, x, y, çap, opaklık]
    readonly property var lightBlobs: [
        ["#d98b73", -160, 360, 760, 0.75],
        ["#f5d6b8", 760, -240, 760, 0.9],
        ["#b9a3c9", 900, 420, 640, 0.55],
        ["#fff1e8", 120, -220, 560, 0.9]
    ]
    readonly property var darkBlobs: [
        ["#7a3a2c", -160, 420, 760, 0.9],
        ["#4a2a4a", 860, 400, 700, 0.75],
        ["#3a2a22", 640, -260, 760, 0.9]
    ]
    readonly property real scaleFactor: width / 1440

    clip: true

    gradient: Gradient {
        GradientStop {
            position: 0
            color: Theme.dark ? "#1c1412" : "#f4e3dc"

            Behavior on color {
                ColorAnimation { duration: Theme.motion.slow }
            }
        }
        GradientStop {
            position: 1
            color: Theme.dark ? "#2b1a16" : "#e2c3b6"

            Behavior on color {
                ColorAnimation { duration: Theme.motion.slow }
            }
        }
    }

    Repeater {
        model: Theme.dark ? root.darkBlobs : root.lightBlobs

        Shape {
            id: blob

            required property var modelData

            // Figma'daki 180 px bulanıklık lekeyi her yönde büyütür, degrade bu yüzden daha
            // geniş bir alana yayılıyor.
            readonly property real diameter: (modelData[3] + 360) * root.scaleFactor
            readonly property color tint: modelData[0]

            x: (modelData[1] - 180) * root.scaleFactor
            y: (modelData[2] - 180) * root.scaleFactor
            width: diameter
            height: diameter

            ShapePath {
                strokeColor: "transparent"
                fillGradient: RadialGradient {
                    centerX: blob.diameter / 2
                    centerY: blob.diameter / 2
                    centerRadius: blob.diameter / 2
                    focalX: centerX
                    focalY: centerY

                    GradientStop {
                        position: 0
                        color: Qt.alpha(blob.tint, blob.modelData[4])
                    }
                    GradientStop {
                        position: 0.45
                        color: Qt.alpha(blob.tint, blob.modelData[4] * 0.75)
                    }
                    GradientStop {
                        position: 1
                        color: Qt.alpha(blob.tint, 0)
                    }
                }

                PathRectangle {
                    width: blob.diameter
                    height: blob.diameter
                    radius: blob.diameter / 2
                }
            }
        }
    }
}
