// Kullanıcının değiştirebildiği kabuk ayarları (karar 0008). Kalıcıdır: Linux'ta
// ~/.config/ankaros/shell.conf, Mac'te ~/Library/Preferences/ankaros/shell.conf dosyasında saklanır.
// Bileşenler ayarları buradan okur, Görünüm ayarları penceresi buraya yazar.
pragma Singleton

import QtCore

Settings {
    // Koyu tema. Theme.dark buna bağlıdır.
    property bool darkMode: false
    // Pencere düğmelerinin başlık çubuğundaki konumu: "right" ya da "left" (karar 0007)
    property string windowButtonSide: "right"
    // Pencere düğmelerinin stili: "icons" (çizgi simgeler) ya da "colored" (renkli daireler)
    property string windowButtonStyle: "icons"

    location: StandardPaths.writableLocation(StandardPaths.ConfigLocation) + "/ankaros/shell.conf"
    category: "appearance"
}
