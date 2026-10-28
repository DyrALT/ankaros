// Masaüstünün kökü: duvar kağıdı, alt çubuk ve başlat paneli.
// Mac önizlemesi bunu bir pencereye, compositor (Faz 1) ise ekranın kendisine yerleştirir.
// Wayland'e özgü hiçbir tür import edilmez. Dış dünyadan gelen veriler property olarak verilir.
import QtQuick

Item {
    id: root

    // Sabitlenmiş uygulamalar, başlat panelinde gösterilir: ["Terminal", ...]
    property var apps: []
    // Alt çubukta gösterilecek uygulamalar (sabitlenmiş + çalışanlar)
    property var taskBarApps: []
    // Penceresi açık uygulamalar ve odaktaki uygulama
    property var running: []
    property string activeApp
    // Başlat panelindeki öneriler: [{ app, title, subtitle }, ...]
    property var recommended: []
    property string userName

    // Başlat panelinin açık olup olmadığı. Compositor'da Super tuşu bunu değiştirecek.
    property alias startMenuOpen: startMenu.open
    // Uygulama pencerelerinin yerleştiği katman: duvar kağıdının üstünde, alt çubuğun altında.
    // Compositor pencere çerçevelerini bu öğenin çocuğu olarak oluşturur.
    readonly property alias windowLayer: windowLayer
    // Pencerelerin kullanabileceği alan (alt çubuk hariç)
    readonly property rect workArea: Qt.rect(0, 0, width, taskBar.y - Theme.size.taskBarMargin)

    // Kullanıcı bir uygulamayı başlattığında. Compositor bunu dinleyip uygulamayı çalıştıracak.
    signal appLaunched(string app)

    // Duvar kağıdı ve pencereler. Alt çubuk ve başlat paneli buzlu cam efekti için bu öğenin
    // görüntüsünü bulanıklaştırır, böylece arkalarındaki pencereler de bulanık görünür.
    Item {
        id: scene

        anchors.fill: parent

        Wallpaper {
            anchors.fill: parent
        }

        Item {
            id: windowLayer

            anchors.fill: parent
        }
    }

    // Başlat paneli açıkken dışarı tıklamak paneli kapatır.
    MouseArea {
        anchors.fill: parent
        enabled: startMenu.open
        onClicked: startMenu.open = false
    }

    StartMenu {
        id: startMenu

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: taskBar.top
        anchors.bottomMargin: Theme.size.taskBarMargin
        backdrop: scene
        apps: root.apps
        recommended: root.recommended
        userName: root.userName
        onAppLaunched: app => {
            open = false;
            root.appLaunched(app);
        }
    }

    TaskBar {
        id: taskBar

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.margins: Theme.size.taskBarMargin
        backdrop: scene
        apps: root.taskBarApps
        running: root.running
        activeApp: root.activeApp
        startMenuOpen: startMenu.open
        onStartClicked: startMenu.open = !startMenu.open
        onSearchClicked: {
            startMenu.open = true;
            startMenu.focusSearch();
        }
        onAppClicked: app => root.appLaunched(app)
    }
}
