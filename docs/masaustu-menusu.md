# Masaüstü sağ tık menüsü

Boş masaüstüne sağ tıklayınca imlecin yerinde bir menü açılır (`DesktopMenu.qml`). Menü şimdilik **sistemde
zaten var olan işlevlerle** kuruldu: yeni servis, sinyal ya da compositor değişikliği yok. İçeriği daha sonra
güncellenecek (yeni maddeler, alt çubuk simgesi ve başlık çubuğu menüleri).

## Davranış

| İş | Nasıl |
|---|---|
| Açılma | Boş masaüstüne sağ düğmeye basınca, imlecin olduğu yerde. Basınca açılır: kısa `TapHandler` tıklamaları yavaş çizimde kaybolabiliyor. |
| Konum | Ekranın ve alt çubuğun dışına taşmaz, kenara yakınsa içeri kayar. |
| Kapanma | Esc, menünün dışına tıklamak, madde seçmek ya da başka bir panelin açılması. Menü açıkken başka yere sağ tık onu kapatır, yenisini açmaz. |
| Pencereler | Pencerenin içindeki sağ tık uygulamaya gider, menü açılmaz. Pencere çerçevesine sağ tık da menü açmaz. |
| Diğer paneller | Başlat paneli, takvim ve hızlı ayarlar açılınca menü kapanır, menü açılınca onlar kapanır. Ekran kilitliyken ya da Alt+Tab sürerken açılmaz. |

## Maddeler

| Madde | Ne yapar | Dayandığı mevcut işlev |
|---|---|---|
| Duvar kağıdını değiştir | Ayarlar'ı Duvar kağıdı sayfasında açar (açıksa o sayfaya geçer) | `settingsRequested("wallpaper")` |
| Görünüm ayarları | Ayarlar'ı Görünüm sayfasında açar | `settingsRequested("appearance")` |
| Koyu/Açık temaya geç | Temayı değiştirir. Ayar kalıcıdır ve bütün uygulamalara yayılır (karar 0015). Etiket şu anki temaya göre değişir | `Appearance.darkMode` |
| Terminal aç | Terminal'i başlat panelindeki gibi açar. Terminal zaten odaktaysa başlat paneli ve alt çubuktaki gibi küçültür | `launch("Terminal")` |
| Ekranı kilitle | Ekranı kilitler (Süper+L ile aynı) | `lock()` |

## Yapı

| Dosya | İş |
|---|---|
| `shell/qml/Ankaros/Shell/DesktopMenu.qml` | Menünün kendisi: maddeler ve sinyaller |
| `shell/qml/Ankaros/Shell/Desktop.qml` | Sağ tıkı yakalar, menüyü yerleştirir, kapanma kuralları, sinyalleri mevcut işlevlere bağlar |
| `shell/qml/Ankaros/UI/MenuItem.qml`, `MenuSeparator.qml` | Menü satırı ve ayırıcı. Güç menüsü de (`PowerMenu.qml`) bunları kullanır |

Kabuk önizlemesinde `--context-menu` bayrağı menüyü açık gösterir (ekran görüntüsü için, CI duman testi de
bunu alır): [tasarim-akisi.md](tasarim-akisi.md).
