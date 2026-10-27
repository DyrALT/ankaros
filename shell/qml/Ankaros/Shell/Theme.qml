// Kabuğun tüm tasarım değerleri. Bileşenler renk, boşluk, yazı boyutu ve süreleri doğrudan yazmaz,
// buradan okur. Adlar Figma değişkenleriyle eşleşir: Figma'da color/surface/panel → color.surfacePanel.
// Değerler Figma tasarımı gelene kadar geçici yer tutuculardır.
pragma Singleton

import QtQuick

QtObject {
    id: theme

    readonly property Colors color: Colors {
        dark: theme.dark
    }
    property bool dark: false
    readonly property FontSizes font: FontSizes {
    }
    readonly property Motion motion: Motion {
    }
    readonly property Radii radius: Radii {
    }
    readonly property Sizes size: Sizes {
    }
    readonly property Spacing spacing: Spacing {
    }

    // Grup türleri. Tipli oldukları için qmllint alanları denetleyebilir, qmlsc binding'leri
    // önceden derleyebilir (tipsiz QtObject ile ikisi de olmaz).
    component Colors: QtObject {
        readonly property color accent: "#6c5ce7"
        readonly property color border: dark ? Qt.rgba(1, 1, 1, 0.10) : Qt.rgba(0, 0, 0, 0.08)
        property bool dark
        readonly property color scrim: dark ? Qt.rgba(0, 0, 0, 0.55) : Qt.rgba(0.1, 0.1, 0.2, 0.35)
        readonly property color surfaceHover: dark ? Qt.rgba(1, 1, 1, 0.08) : Qt.rgba(0, 0, 0, 0.06)
        readonly property color surfacePanel: dark ? Qt.rgba(0.11, 0.11, 0.16, 0.78) : Qt.rgba(1, 1,
                                                                                               1, 0.72)
        readonly property color surfacePressed: dark ? Qt.rgba(1, 1, 1, 0.14) : Qt.rgba(0, 0, 0,
                                                                                        0.11)

        readonly property color textOnScrim: "#ffffff"
        readonly property color textPrimary: dark ? "#f2f2f7" : "#1c1c28"
        readonly property color textSecondary: dark ? "#a1a1b5" : "#5c5c70"
        readonly property color wallpaperBottom: dark ? "#1d1430" : "#f3e7ff"
        readonly property color wallpaperTop: dark ? "#0f1222" : "#c9d6ff"
    }
    component FontSizes: QtObject {
        readonly property int body: 14
        readonly property int display: 40
        readonly property int small: 12
        readonly property int title: 22
    }
    component Motion: QtObject {
        readonly property int easing: Easing.OutCubic
        readonly property int fast: 120
        readonly property int normal: 200
        readonly property int slow: 320
    }
    component Radii: QtObject {
        readonly property int lg: 20
        readonly property int md: 12
        readonly property int sm: 8
    }
    component Sizes: QtObject {
        readonly property int appIcon: 64
        readonly property int appTile: 112
        readonly property int panelButton: 40
        readonly property int panelHeight: 56
    }
    component Spacing: QtObject {
        readonly property int lg: 16
        readonly property int md: 12
        readonly property int sm: 8
        readonly property int xl: 24
        readonly property int xs: 4
        readonly property int xxl: 48
    }
}
