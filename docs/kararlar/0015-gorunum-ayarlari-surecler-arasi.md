# 0015: Görünüm ayarları süreçler arasında paylaşılır

- **Durum:** Kabul edildi
- **Tarih:** 2026-10-28

## Bağlam

Karar 0008'de görünüm ayarları (tema, pencere düğmeleri) kabuktaki QtCore `Settings` tekil nesnesinde
(`ShellSettings`) tutuluyordu. Faz 3'te Ayarlar ayrı bir uygulama oluyor (karar 0013): ayarı Ayarlar
uygulaması değiştirecek, masaüstü ve açık uygulamalar değişikliği hemen görmeli. QtCore `Settings` dosyayı
yalnızca açılışta okur, başka bir sürecin yaptığı değişikliği fark etmez.

## Değerlendirilen seçenekler

- **D-Bus ile ayar servisi (dconf gibi):** Değişiklik bildirimi hazır, ama D-Bus'ı sisteme eklemeyi
  gerektirir (karar 0010 ve 0011'de bundan kaçınıldı).
- **Masaüstünün ayar soketi:** Ayarlar uygulaması değişikliği compositor'a sokete yazar. Ama masaüstü
  kapalıyken ya da Mac önizlemesinde çalışmaz.
- **Aynı dosya, izlenerek:** Ayarlar aynı INI dosyasında kalır, her süreç dosyayı `QFileSystemWatcher`
  ile izler ve değişince yeniden okur.

## Karar

Ayarlar aynı dosyada kalır: `~/.config/ankaros/shell.conf`, `[appearance]` bölümü. Ankaros.UI'deki C++
`Appearance` tekil nesnesi dosyayı okur, yazar ve izler. Bileşenler `Appearance.darkMode` gibi özellikleri
kullanır, `Theme.dark` buna bağlıdır. 0008'deki saklama yeri ve biçimi değişmez, var olan ayarlar korunur.

## Sonuçlar

- Ayarlar uygulaması, masaüstü ve diğer uygulamalar aynı ayarı anında paylaşır. Masaüstü kapalıyken de
  çalışır.
- QSettings dosyayı yeni dosya yazıp yerine taşıyarak kaydettiği için dosyanın yanında klasör de izlenir.
- Yeni ayarlar (duvar kağıdı, alt çubuk) aynı yolla eklenir.
