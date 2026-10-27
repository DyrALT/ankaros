// Başlatıcı simgesi: 3x3 nokta ızgarası. Figma'dan SVG simgeler gelince değiştirilecek.
import QtQuick

Grid {
    columns: 3
    spacing: 4

    Repeater {
        model: 9

        Rectangle {
            color: Theme.color.textPrimary
            height: 5
            radius: 2.5
            width: 5
        }
    }
}
