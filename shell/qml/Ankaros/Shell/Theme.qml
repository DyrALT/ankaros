// Kabuğun tüm tasarım değerleri. Bileşenler renk, boşluk, yazı boyutu ve süreleri doğrudan yazmaz,
// buradan okur. Adlar Figma'daki "Theme" ve "Vurgu" değişken koleksiyonlarıyla eşleşir:
// Figma'da color/surface/panel → color.surfacePanel, radius/lg → radius.lg.
// Kaynak: "ankaros: Masaüstü Tasarımı", B · Ankara Taşı varyasyonu.
pragma Singleton

import QtQuick

QtObject {
    id: theme

    // Kullanıcının ayarından gelir. Önizleme ekran görüntüleri için doğrudan da atanabilir.
    property bool dark: ShellSettings.darkMode

    readonly property Colors color: Colors {
        dark: theme.dark
    }
    readonly property Spacing spacing: Spacing {}
    readonly property Radii radius: Radii {}
    readonly property Fonts font: Fonts {}
    readonly property Sizes size: Sizes {}
    readonly property Motion motion: Motion {}

    // Inter, modülün içine gömülü (fonts/, SIL Open Font License). Dört ağırlık da aynı aileye
    // ("Inter") kaydolur, Label bileşeni ağırlığı font.weight ile seçer.
    readonly property list<FontLoader> fontLoaders: [
        FontLoader { source: "fonts/Inter-Regular.ttf" },
        FontLoader { source: "fonts/Inter-Medium.ttf" },
        FontLoader { source: "fonts/Inter-SemiBold.ttf" },
        FontLoader { source: "fonts/Inter-Bold.ttf" }
    ]

    // Grup türleri. Tipli oldukları için qmllint alanları denetleyebilir, qmlsc binding'leri
    // önceden derleyebilir (tipsiz QtObject ile ikisi de olmaz).
    component Colors: QtObject {
        property bool dark

        readonly property color surfacePanel: dark ? Qt.rgba(0.118, 0.110, 0.106, 0.62)
                                                   : Qt.rgba(1, 1, 1, 0.62)
        readonly property color surfaceWindow: dark ? "#201e1d" : "#fbfaf8"
        readonly property color surfaceSidebar: dark ? "#282524" : "#f2efeb"
        readonly property color surfaceHover: dark ? Qt.rgba(1, 1, 1, 0.08) : Qt.rgba(0, 0, 0, 0.05)
        readonly property color surfacePressed: dark ? Qt.rgba(1, 1, 1, 0.14) : Qt.rgba(0, 0, 0, 0.10)
        readonly property color border: dark ? Qt.rgba(1, 1, 1, 0.10) : Qt.rgba(0, 0, 0, 0.08)
        readonly property color scrim: dark ? Qt.rgba(0, 0, 0, 0.50) : Qt.rgba(0.078, 0.063, 0.047, 0.30)
        readonly property color textPrimary: dark ? "#f5f2ee" : "#1d1b19"
        readonly property color textSecondary: dark ? "#a39c95" : "#6b6560"
        readonly property color textOnAccent: "#ffffff"

        // Renkli daire stilindeki pencere düğmeleri (color/windowButton/*)
        readonly property color windowClose: "#e5604a"
        readonly property color windowMinimize: "#f2b33d"
        readonly property color windowMaximize: "#5fb37a"
        readonly property color windowButtonInactive: dark ? Qt.rgba(1, 1, 1, 0.18) : Qt.rgba(0, 0, 0, 0.14)

        // Vurgu koleksiyonu, "Ankara Taşı" modu
        readonly property color accent: "#c0674e"
        readonly property color accentSoft: Qt.rgba(0.753, 0.404, 0.306, 0.16)
    }

    component Spacing: QtObject {
        readonly property int xs: 4
        readonly property int sm: 8
        readonly property int md: 12
        readonly property int lg: 16
        readonly property int xl: 24
        readonly property int xxl: 48
    }

    component Radii: QtObject {
        readonly property int sm: 8
        readonly property int md: 12
        readonly property int lg: 20
        readonly property int xl: 28
    }

    component Fonts: QtObject {
        readonly property string family: "Inter"
        readonly property int caption: 11
        readonly property int small: 12
        readonly property int body: 13
        readonly property int large: 14
        readonly property int title: 22
        readonly property int display: 40
    }

    component Sizes: QtObject {
        readonly property int taskBar: 56
        readonly property int taskBarMargin: 12
        readonly property int startMenuWidth: 680
        readonly property int panelButton: 40
        readonly property int taskIcon: 34
        readonly property int pinnedIcon: 48
        readonly property int pinnedTile: 105
        readonly property int statusIcon: 16
    }

    component Motion: QtObject {
        readonly property int fast: 120
        readonly property int normal: 200
        readonly property int slow: 320
        readonly property int easing: Easing.OutCubic
    }
}
