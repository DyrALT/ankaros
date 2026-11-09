// Kurulan (üçüncü taraf) uygulamaların kendi simgeleri: uygulama adından resim adresine tablo.
// Ankaros.UI sistem servislerini bilmez, tabloyu AppCatalog.icons'tan kabuğu ya da uygulamayı
// başlatan taraf doldurur (compositor/Main.qml, Ayarlar).
pragma Singleton

import QtQuick

QtObject {
    property var map: ({})
}
