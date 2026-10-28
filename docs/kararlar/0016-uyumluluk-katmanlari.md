# 0016: Başka sistemlerin uygulamaları için uyumluluk katmanları

- **Durum:** Önerildi
- **Tarih:** 2026-10-29

## Bağlam

Kullanıcı, uygulamaların kendi sitelerinden indirdiği paketleri ankaros'ta çalıştırabilmeli: VS Code'un
`.deb`'i, bir başka uygulamanın `.rpm`'i, AppImage dosyaları. İleride Windows `.exe` dosyaları da
çalışmalı.

Bu paketler belirli bir dağıtımın (Debian, Fedora) kütüphane sürümleri, dosya yerleşimi ve bağımlılık
deposu varsayılarak hazırlanmıştır. ankaros Buildroot ile kendi sürümlerimizden kurulur, bu varsayımların
hiçbirini karşılamaz. Paketin dosyalarını kopyalamak ya da bağımlılıklarını tek tek taşımak çoğu
uygulamada çalışmaz.

Kısıtlar:

- Temel sistem küçük, hızlı ve bozulmaz kalmalı.
- Şu an arm64 için geliştiriliyor, x86_64 desteği sonra gelecek. Mimari işlemciden bağımsız olmalı.
- Kullanıcı bir paketi çift tıklayarak ya da Paket Merkezi'nden kurabilmeli, uygulama başlat panelinde
  diğerleri gibi görünmeli.

## Değerlendirilen seçenekler

- **Kendi paket biçimimiz (karar 0012):** Küçük ve tamamen bizim kontrolümüzde. Ama kullanıcı yalnızca
  bizim paketlediğimiz uygulamaları kurabilir, `.deb` ve `.rpm` çalışmaz. Senaryoyu karşılamıyor.
- **Hazır bir paket aracı (apk-tools, opkg):** İmza ve kurulum kodu hazır. Ama paketleri yine biz
  üretiriz, başka dağıtımların paketleri çalışmaz. Senaryoyu karşılamıyor.
- **Paketleri açıp dosyalarını sisteme kopyalamak:** Bağımlılıklar çözülmez, kütüphane sürümleri tutmaz,
  temel sistem kirlenir. Güvenilmez.
- **Flatpak:** Flathub'daki binlerce uygulama, yalıtım. Ama `.deb` ve `.rpm` çalışmaz, yalnızca
  Flatpak olarak yayımlanan uygulamalar kurulur. D-Bus, portallar ve ostree ister.
- **Uyumluluk katmanları (kapsayıcılar):** Her paket türü için o dağıtımın gerçek kullanıcı alanını taşıyan
  hafif bir kapsayıcı: `.deb` Debian'da `apt` ile, `.rpm` Fedora'da `dnf` ile kurulur. Bağımlılıkları o
  dağıtımın kendi deposu çözer. Uygulama ankaros'un ekranını, sesini, GPU'sunu ve ev klasörünü kullanır.
  ChromeOS'un Linux desteği ve Distrobox bu yöntemi kullanır. Uyumluluk çok yüksek. Kapsayıcı ilk
  kullanımda indirilir (birkaç yüz MB).

## Karar

Uyumluluk katmanları. Kendi paket biçimimizden (karar 0012) vazgeçiyoruz.

- Temel sistem imaj olarak kalır. Terminal, Ayarlar ve Paket Merkezi imajın parçasıdır.
- Her katman, bir dağıtımın resmî kök dosya sistemiyle kurulan bir kapsayıcıdır. Kapsayıcılar sistem
  genelindedir (`/var/lib/ankaros/katmanlar/<ad>`), kullanıcının ilk ihtiyacında internetten indirilir,
  imaja girmez.
- Kapsayıcıyı kendi küçük başlatıcımız Linux'un ad alanlarıyla (namespace) çalıştırır. Docker ya da
  Podman kullanılmaz.
- Paket türü dosyadan anlaşılır ve doğru katmana yönlendirilir:
  - `.deb` → Debian
  - `.rpm` → Fedora
  - AppImage → Debian katmanında çalışır
  - `.exe` → Windows katmanı (Wine), x86_64 desteğinden sonra
- Kurulan uygulamanın `.desktop` dosyası ve simgesi ankaros'un uygulama listesine eklenir. Uygulama
  başlat panelinde ve alt çubukta diğerleri gibi görünür, compositor'da kendi penceresiyle açılır.
- Kurma, kaldırma ve katman yönetimi root olarak çalışan bir serviste toplanır. `ank` komutu ve
  Paket Merkezi bu servisin soketine bağlanır (diğer servislerimizdeki gibi).

## Sonuçlar

- Kullanıcı, ankaros için ayrıca paketlenmemiş uygulamaları kurabilir. Uyumluluk, paketin yazıldığı
  dağıtımdakine yakın olur.
- İlk `.deb` kurulumunda Debian katmanı indirilir (yaklaşık 200-300 MB). İnternet gerekir.
- Katmanlar kendi kütüphanelerini taşır. Aynı kütüphane hem ankaros'ta hem katmanda bulunur, disk
  kullanımı artar. Temel sistem bundan etkilenmez.
- Katman uygulamaları ankaros'un tasarım dilinde değildir. Pencere çerçevesini ankaros çizer, içeriği
  uygulamanın kendisidir.
- Gerekli altyapı:
  - Ses: PipeWire
  - D-Bus
  - X11 uygulamaları için XWayland
  - Compositor'ın desteklemediği Wayland protokolleri

  Bunlar katmanlardan önce gelmeli.
- İşlemci mimarisi: arm64'te yalnızca arm64 paketleri çalışır (VS Code'un arm64 `.deb`'i var).
  Yalnızca x86_64 için yayımlanan uygulamalar ve Windows programları x86_64 desteği gelince çalışır.
  ARM'de x86 çevirmeni (FEX-Emu, box64) planlanmıyor.
- Flatpak bu kararı dışlamaz. İleride ayrı bir katman olarak eklenebilir.
- Katmanlar root olmayan kullanıcının uygulamalarını yalıtmaz (sandbox değildir). Uygulama, kullanıcının
  ev klasörüne erişir. Bu, dağıtımlarda paket kurmakla aynı güven düzeyidir.
