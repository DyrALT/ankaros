// Varsayılan duvar kağıdı: tema renklerinden dikey bir geçiş.
import QtQuick

Rectangle {
    gradient: Gradient {
        GradientStop {
            color: Theme.color.wallpaperTop
            position: 0

            Behavior on color {
                ColorAnimation {
                    duration: Theme.motion.slow
                }
            }
        }
        GradientStop {
            color: Theme.color.wallpaperBottom
            position: 1

            Behavior on color {
                ColorAnimation {
                    duration: Theme.motion.slow
                }
            }
        }
    }
}
