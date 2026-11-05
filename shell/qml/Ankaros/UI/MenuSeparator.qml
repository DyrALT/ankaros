// Menü satırları arasındaki ince ayırıcı çizgi. Bir ColumnLayout içinde durur.
import QtQuick
import QtQuick.Layouts

Rectangle {
    Layout.fillWidth: true
    Layout.preferredHeight: 1
    Layout.topMargin: 2
    Layout.bottomMargin: 2
    color: Theme.color.border
}
