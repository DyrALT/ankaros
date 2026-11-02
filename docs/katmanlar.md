# Uyumluluk katmanları

Başka dağıtımların uygulamaları, o dağıtımın kendi kök dosya sisteminde, yani bir **katmanda** çalışır
([karar 0016](kararlar/0016-uyumluluk-katmanlari.md)). Katmanlar veri diskindedir
([karar 0017](kararlar/0017-veri-diski.md)): `/var/lib/ankaros/katmanlar/<ad>/kok`.

## Komutlar

```bash
ank katman liste                                   # kurulu katmanlar, sürümleri ve boyutları
ank katman kur debian                              # Debian 13 katmanını indirir ve kurar (root)
ank katman calistir debian -- apt install foot     # root: katmana paket kurar
ank katman calistir debian -- foot                 # kullanıcı: katmandaki uygulamayı açar
ank katman sil debian                              # katmanı ve içine kurulanları siler (root)
```

## Kurulum

- Kaynak linuxcontainers.org'un hazır kök dosya sistemleridir: `images/debian/trixie/arm64/default/<yapım>/`.
  En yeni yapım seçilir. `rootfs.tar.xz`'in SHA-256 özeti aynı klasördeki `SHA256SUMS` ile denetlenir, ikisi
  de HTTPS üzerinden gelir. Özet tutmazsa katman kurulmaz.
- Başka bir kaynak `ANK_KAYNAK` ortam değişkeniyle verilebilir (ör. testte yerel bir HTTP sunucusu).
- Debian katmanı açılınca yaklaşık 450 MB.

## Çalıştırma: `ankaros-katman`

`system/katman/`. Komutu katmanın içinde çalıştırır:

- **Kullanıcı olarak:** Yeni bir kullanıcı ad alanı ve bağlama ad alanı açılır (`CONFIG_USER_NS`, çekirdek
  yapılandırma parçası `external/board/qemu-aarch64/linux.fragment`). Kullanıcı kendi kimliğiyle kalır,
  root yetkisi gerekmez, setuid program yoktur. Katmanın root'a ait dosyaları `nobody` görünür.
- **Root olarak:** Yalnızca bağlama ad alanı açılır. `apt` gibi paket yöneticileri böyle çalışır.
- Katmanda `/run` ve `/home` boş birer tmpfs'tir. ankaros'tan paylaşılanlar (`layer.h`, `sharedPaths`):
  - `/dev`, `/proc`, `/sys`, `/tmp`
  - `/etc/resolv.conf` (ad çözümü)
  - kullanıcının ev klasörü ve `XDG_RUNTIME_DIR` (Wayland soketi)
- Katman bir sandbox değildir: uygulama kullanıcının dosyalarına erişir, dağıtımlarda paket kurmakla aynı.

## Bilinen eksikler

- **Compositor'ın eksik protokolleri (Faz 4 B):** `foot` penceresini açıyor ama compositor seçili metni orta
  tuşla yapıştırma, `xdg-activation`, kesirli ölçek, imleç şekli ve `text-input` sunmuyor. `linux-dmabuf`
  henüz açılmadı, uygulamalar ekran belleğini paylaşılan bellekle gönderiyor.
- `ankaros-katman` ortam değişkenlerini temizlemiyor. ankaros'un bash biçimindeki `PS1`'i katmandaki
  `/bin/sh`'a geçerse istem bozuk görünür.
- Katmanda Türkçe yerel ayar (`tr_TR.UTF-8`) yok, uygulamalar `C.UTF-8`'e düşüyor. Katman kurulurken
  `locales` eklenecek.
- ankaros'un yazı tipleri katmanla paylaşılmıyor, uygulamalar Debian'ın yazı tiplerini kullanıyor.
