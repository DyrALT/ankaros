// Masaüstünün kökü: duvar kağıdı, panel ve uygulama başlatıcı.
// Mac önizlemesi bunu bir pencereye, compositor (Faz 1) ise ekranın kendisine yerleştirir.
// Wayland'e özgü hiçbir tür import edilmez. Dış dünyadan gelen veriler property olarak verilir.
import QtQuick

Item {
    id: root

    // Başlatıcıda gösterilecek uygulamalar: [{ name: "Terminal", color: "#..." }, ...]
    // Önizlemede sahte veri, ankaros'ta sistemdeki .desktop dosyalarından gelecek.
    property var apps: []

    // Başlatıcının açık olup olmadığı. Compositor'da Super tuşu bunu değiştirecek.
    property alias launcherOpen: launcher.open

    Wallpaper {
        anchors.fill: parent
    }
    Panel {
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Theme.spacing.lg
        anchors.horizontalCenter: parent.horizontalCenter
        launcherOpen: launcher.open

        onLauncherToggled: launcher.open = !launcher.open
    }
    Launcher {
        id: launcher

        anchors.fill: parent
        apps: root.apps
    }
}
