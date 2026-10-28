# 0008: Kullanıcı ayarları QtCore Settings ile ~/.config/ankaros altında

- **Durum:** Kabul edildi
- **Tarih:** 2026-10-28

## Bağlam

Faz 1 ile birlikte kullanıcının değiştirebileceği ilk ayarlar geliyor: pencere düğmelerinin konumu ve
stili, açık/koyu mod. Ayarlar yeniden başlatmada korunmalı, değişiklik anında uygulanmalı ve Mac
önizlemesinde de çalışmalı.

## Değerlendirilen seçenekler

- **Kendi JSON dosyamız + C++ okuyucu:** Tam kontrol, ama dosya izleme, varsayılanlar ve hata yönetimini
  kendimiz yazarız.
- **dconf/GSettings:** GNOME ekosisteminin standardı, ama GLib bağımlılığı getirir.
- **QtCore `Settings` (QML) / `QSettings` (C++):** Qt'nin içinde, QML'den doğrudan bağlanabilir,
  platformun standart yerinde INI dosyası olarak saklar. Linux'ta `~/.config/ankaros/shell.conf`,
  Mac'te `~/Library/Preferences` altında.

## Karar

QtCore `Settings`. Ayarlar Ankaros.Shell modülündeki tek bir `ShellSettings` tekil nesnesinde toplanır,
bileşenler ayarları oradan okur. Dosya adı ve kategori: `ankaros/shell`, bölüm `[appearance]`.

## Sonuçlar

- Ayarlar şimdilik kabuk sürecine özel. İleride başka uygulamalar (Faz 3'teki Ayarlar uygulaması) aynı
  ayarları değiştirmek isterse, kabuk bir D-Bus arayüzü sunabilir ya da dosya değişikliklerini izleyebilir.
- Sistem genelindeki varsayılanlar ileride `/etc/xdg/ankaros/shell.conf` ile verilebilir (QSettings bunu
  zaten destekliyor).
