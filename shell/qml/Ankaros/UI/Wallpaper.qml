// Duvar kağıdı: dikey bir renk geçişi üzerinde yumuşak renk lekeleri. Dört palet var (Ayarlar › Duvar
// kağıdı), her birinin açık ve koyu temada ayrı tonları vardır.
// Lekeler Figma'daki bulanıklaştırılmış elipslerin karşılığıdır. Bulanıklık efekti yerine radyal
// degrade kullanıldığı için çizimi ucuzdur. Konumlar 1440x900'lük tasarıma göre verilip ekrana
// ölçeklenir.
pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Shapes

Rectangle {
    id: root

    // Paletin adı: "ankara-tasi", "bozkir", "kehribar", "gece". Varsayılan kullanıcının seçimi.
    property string name: Appearance.wallpaper
    // Her palet için açık ve koyu temada: üst ve alt renk, lekeler [renk, x, y, çap, opaklık]
    readonly property var palettes: ({
            "ankara-tasi": {
                light: { top: "#f4e3dc", bottom: "#e2c3b6", blobs: [
                        ["#d98b73", -160, 360, 760, 0.75], ["#f5d6b8", 760, -240, 760, 0.9],
                        ["#b9a3c9", 900, 420, 640, 0.55], ["#fff1e8", 120, -220, 560, 0.9]] },
                dark: { top: "#1c1412", bottom: "#2b1a16", blobs: [
                        ["#7a3a2c", -160, 420, 760, 0.9], ["#4a2a4a", 860, 400, 700, 0.75],
                        ["#3a2a22", 640, -260, 760, 0.9]] }
            },
            "bozkir": {
                light: { top: "#f3ead6", bottom: "#e2cfa8", blobs: [
                        ["#d4b06a", -160, 360, 760, 0.7], ["#f6e9c9", 760, -240, 760, 0.9],
                        ["#9fb08a", 900, 420, 640, 0.55], ["#fffaf0", 120, -220, 560, 0.9]] },
                dark: { top: "#17150f", bottom: "#2a2416", blobs: [
                        ["#6b5428", -160, 420, 760, 0.85], ["#3d4a30", 860, 400, 700, 0.7],
                        ["#3a3020", 640, -260, 760, 0.9]] }
            },
            "kehribar": {
                light: { top: "#fbe8cf", bottom: "#efb57e", blobs: [
                        ["#e8873e", -160, 360, 760, 0.7], ["#fde3b0", 760, -240, 760, 0.9],
                        ["#d9604a", 900, 420, 640, 0.5], ["#fff4e0", 120, -220, 560, 0.9]] },
                dark: { top: "#1d120a", bottom: "#3a1f10", blobs: [
                        ["#8a4a1e", -160, 420, 760, 0.9], ["#5a2a1a", 860, 400, 700, 0.75],
                        ["#40280f", 640, -260, 760, 0.9]] }
            },
            "gece": {
                light: { top: "#e3e6f2", bottom: "#b9bfdc", blobs: [
                        ["#7f8bc4", -160, 360, 760, 0.6], ["#dfe3f5", 760, -240, 760, 0.9],
                        ["#a58bc4", 900, 420, 640, 0.55], ["#f4f5fb", 120, -220, 560, 0.9]] },
                dark: { top: "#0f1220", bottom: "#1c1f36", blobs: [
                        ["#2d3466", -160, 420, 760, 0.9], ["#4a2d5c", 860, 400, 700, 0.75],
                        ["#1f2a4a", 640, -260, 760, 0.9]] }
            }
        })
    readonly property var colors: {
        const set = palettes[name] ?? palettes["ankara-tasi"];
        return Theme.dark ? set.dark : set.light;
    }
    readonly property real scaleFactor: width / 1440

    clip: true

    gradient: Gradient {
        GradientStop {
            position: 0
            color: root.colors.top

            Behavior on color {
                ColorAnimation { duration: Theme.motion.slow }
            }
        }
        GradientStop {
            position: 1
            color: root.colors.bottom

            Behavior on color {
                ColorAnimation { duration: Theme.motion.slow }
            }
        }
    }

    Repeater {
        model: root.colors.blobs

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
