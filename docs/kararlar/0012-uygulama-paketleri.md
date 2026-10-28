# 0012: Kendi uygulama paketimiz

- **Durum:** Yerine geçen: [0016](0016-uyumluluk-katmanlari.md)
- **Tarih:** 2026-10-28

## Bağlam

Faz 2'ye kadar her şey imajın içindeydi: yeni bir uygulama için imajın yeniden derlenmesi gerekiyordu.
Faz 3'te uygulamalar imajdan bağımsız olarak kurulup kaldırılabilmeli. İstekler:

- Paketlenen program herhangi bir dilde yazılmış olabilmeli, başkalarının uygulamaları da paketlenebilmeli.
- Sistemde olmayan kütüphaneler paketle birlikte gelebilmeli.
- Paket Merkezi ve kurulum bizim tasarım dilimizde ve bizim kontrolümüzde olmalı.
- Sistem küçük ve hızlı kalmalı.

## Değerlendirilen seçenekler

- **Flatpak:** Linux masaüstünün yaygın biçimi, Flathub'daki binlerce uygulama. Ama ostree, bubblewrap,
  D-Bus ve XDG portalları ister. İmajı ve açılışı büyütür, kurulum bizim arayüzümüzün dışında kalır.
- **opkg:** Buildroot'ta hazır, hafif. Sistemin parçalarını da güncelleyebilir. Ama bağımlılık çözümü,
  depo yönetimi ve paket üretim hattı bizim için ek yük. Asıl amacı uygulama değil sistem paketleri.
- **Kendi biçimimiz:** Bir manifest ve dosyalardan oluşan, zstd ile sıkıştırılmış tar arşivi. Kütüphaneler
  paketin içinde gelebilir (AppImage'a benzer). Kurulum `/opt/ankaros/apps/<id>` altına, imzalı bir
  depodan. Küçük, ama indirme, imza ve kurulum kodunu kendimiz yazarız.

## Karar

Kendi uygulama paketimiz: `.ankpkg`. Ayrıntılar: [faz-3-plani.md](../faz-3-plani.md).

- Sistem imaj olarak kalır, yalnızca uygulamalar paketlenir.
- Depo statik dosyalardan oluşur, paket listesi ed25519 ile imzalanır.
- İndirme, imza denetimi ve açma işini root olarak çalışan `ankaros-packages` servisi yapar. Kullandığı
  kütüphaneler libcurl, libsodium ve libarchive, hepsi Buildroot'ta hazır ve MIT ile uyumlu.
- `ank` komutu ve Paket Merkezi bu servisin soketine bağlanır.

## Sonuçlar

- Wayland'li her Linux uygulaması, aarch64 için derlenip paketlenirse çalışır. Dağıtımların `.deb` ve
  `.rpm` paketleri doğrudan kurulamaz. Yalnızca X11 kullanan uygulamalar çalışmaz (XWayland yok).
- Flatpak ileride ikinci bir kaynak olarak eklenebilir. Bu karar onu dışlamaz.
- Sistemin kendisinin güncellenmesi (A/B bölüm, imaj güncellemesi) ayrı bir karar olacak.
- İmza anahtarı bir güvenlik varlığıdır: özel anahtar yalnızca CI'da tutulur.
