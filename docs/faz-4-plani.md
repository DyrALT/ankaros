# Faz 4 planı: başka sistemlerin uygulamaları (`v0.5.0`)

**Hedef:** Kullanıcı, uygulamanın sitesinden indirdiği `.deb`'i (ilk örnek VS Code) çift tıklayarak ya da
Paket Merkezi'nden kurar. Uygulama başlat panelinde görünür, ankaros'ta kendi penceresinde açılır,
sesi ve GPU'yu kullanır. Ardından aynı altyapıyla `.rpm` ve AppImage gelir.

Karar: [0016: Başka sistemlerin uygulamaları için uyumluluk katmanları](kararlar/0016-uyumluluk-katmanlari.md)

## Mimari

```
ankaros (küçük, değişmez temel sistem)
├── ankaros-compositor        Wayland, XWayland (X11 uygulamaları için)
├── PipeWire                  ses
├── D-Bus                     uygulamalar arası mesajlaşma
└── ankaros-apps (root)       katmanları indirir, kurar, kaldırır, uygulama listesini günceller
    ├── ank                   terminal arayüzü: ank kur ./code.deb, ank liste, ank kaldır code
    └── Paket Merkezi         dosyadan kurma, kurulu uygulamalar, katmanlar

/var/lib/ankaros/katmanlar/
├── debian/                   Debian'ın kök dosya sistemi: .deb ve AppImage
└── fedora/                   Fedora'nın kök dosya sistemi: .rpm

Katmandaki uygulama ankaros'la şunları paylaşır:
  Wayland ve X11 soketleri, PipeWire, D-Bus, GPU (/dev/dri), ev klasörü, yazı tipi ayarları, saat dilimi
```

### Kurulum akışı (`.deb`)

1. Kullanıcı `code_arm64.deb`'i çift tıklar ya da `ank kur ./code_arm64.deb` yazar.
2. `ankaros-apps` dosyanın türüne bakar (`.deb`) ve Debian katmanını seçer.
3. Katman yoksa indirir:
   - Debian'ın resmî kök dosya sistemi, imzası denetlenerek
   - Yaklaşık 200-300 MB, ilerleme Paket Merkezi'nde görünür
4. Katmanın içinde `apt install ./code_arm64.deb` çalışır. Bağımlılıkları Debian'ın deposu çözer.
5. Paketin getirdiği `.desktop` dosyaları ve simgeler okunur, ankaros'un uygulama listesine eklenir.
6. VS Code başlat panelinde görünür. Tıklanınca katmanın içinde başlatılır.

### Kapsayıcı başlatıcısı

- Kendi küçük C++ programımız. Bağımlılık eklemez, Docker ya da Podman kullanılmaz.
- Yeni bir bağlama ad alanında (mount namespace) katmanın kök dosya sistemine geçer, paylaşılan
  klasörleri bağlar. Uygulamayı kullanıcının kimliğiyle başlatır.
- `apt` ve `dnf` katmanın içinde root olarak çalışır. Bunu yalnızca `ankaros-apps` servisi yapar.
- Katmanlar sandbox değildir: uygulama kullanıcının dosyalarına erişir (dağıtımlarda paket kurmakla aynı).

## Adımlar

Her adım ayrı bir PR'dır. PR'lar üst üste dizilmez.

| PR | İçerik |
|---|---|
| **A** `feat(system)` | Ses: PipeWire ve WirePlumber, alt çubuktaki ses simgesi gerçek ses düzeyini gösterir. D-Bus (oturum ve sistem veri yolu). |
| **B** `feat(compositor)` | Eksik Wayland protokolleri (aşağıdaki liste), dış uygulamaların GPU arabellekleri (linux-dmabuf). Wayland'li bir Debian uygulamasıyla denenir. |
| **C** `feat(system)` | Veri diski ([karar 0017](kararlar/0017-veri-diski.md)), kullanıcı ad alanları. Kapsayıcı başlatıcısı ve Debian katmanı: indirme, imza denetimi, `ank katman` komutları. Katmanda `glxgears` ve bir GTK uygulaması ankaros'ta açılır. |
| **D** `feat(system)` | `ankaros-apps` servisi ve `ank`: `.deb` kurma ve kaldırma, uygulama listesine ekleme. **Hedef: VS Code çalışır.** |
| **E** `design` | Figma: Paket Merkezi'nin yeni hali (dosyadan kurma, kurulu uygulamalar, katmanlar, ilerleme). **Onayın alınır.** |
| **F** `feat(apps)` | Paket Merkezi. `.deb` dosyasına çift tıklama (dosya yöneticisi yokken Terminal'den `ank kur`). |
| **G** `feat(compositor)` | XWayland (yalnızca X11 bilen uygulamalar için) |
| **H** `feat(system)` | Fedora katmanı (`.rpm`) ve AppImage |
| **I** `chore` | `v0.5.0` |

A, B ve C birbirinden bağımsızdır, sırayla ama birbirini beklemeden yapılabilir. D, C'den sonra gelir.

## A adımı: ses ve D-Bus (ayrıntılı plan)

Durum (2026-10-29): Buildroot'ta PipeWire 1.2.8, WirePlumber 0.5.10, D-Bus 1.14 ve ALSA hazır. Çekirdekte
ses desteği yok, QEMU'da ve UTM'de ses aygıtı tanımlı değil.

1. **Tasarım:** Figma'da "Faz 3 · Masaüstü cilası" sayfasındaki Hızlı ayarlar ekranına ses kaydırıcısı ve alt
   çubuğa ses simgesi (ses aygıtı yoksa görünmez). **Onayın alınır.**
2. **Çekirdek ve sanal makineler:**
   - Çekirdek yapılandırma parçası (`BR2_LINUX_KERNEL_CONFIG_FRAGMENT_FILES`): `CONFIG_SND`,
     `CONFIG_SND_VIRTIO`, `CONFIG_SND_HDA_INTEL` ve genel HDA kodeği.
   - `run-qemu.sh` ve `qemu-test.py`: `-device virtio-sound-pci` ile ses aygıtı. Mac'te `coreaudio`, CI'da
     `none` ses sürücüsü.
   - `utm-olustur.sh`: UTM'ye ses aygıtı.
3. **Paketler:** `dbus`, `pipewire`, `wireplumber` (Lua 5.4), `alsa-lib`, denemeler için `alsa-utils`.
4. **Servisler:**
   - Sistem D-Bus'ı ankaros-init'te bir servis.
   - Oturum yöneticisi her oturumda kullanıcının D-Bus'ını (`dbus-daemon --session`), PipeWire'ı ve
     WirePlumber'ı başlatır, oturum kapanınca durdurur.
   - Compositor ve uygulamalar `DBUS_SESSION_BUS_ADDRESS`'i alır.
5. **Ses denetimi:**
   - `Ankaros.System`'e `Audio` sınıfı: varsayılan çıkışın ses düzeyi ve sessiz durumu, okuma ve değiştirme.
   - İlk sürüm `wpctl` üzerinden çalışır, çıktısını çözümleyen kısma birim testi yazılır.
   - Mac önizlemesi için sahte `Audio.qml`.
6. **Arayüz:** Hızlı ayarlarda ses kaydırıcısı ve sessiz düğmesi, alt çubukta ses simgesi ve düzey balonu.
7. **Test:**
   - QEMU: seri konsoldan `wpctl status` ile ses aygıtının görünmesi, kaydırıcının düzeyi değiştirmesi.
   - UTM: gerçek ses.

Tahmini imaj büyümesi 10-15 MB. Çekirdeğin ve PipeWire'ın derlenmesi uzun sürer.

## Compositor'ın eksikleri

Dış uygulamalar Qt 6.9'un compositor'ının sunduğundan yeni protokoller isteyebilir (Faz 1'de foot
terminali bu yüzden açılmamıştı). B adımında ölçülecek, gerekenler eklenecek:

| Protokol | Ne için | Durum |
|---|---|---|
| `wl_seat` sürüm 5+, `wl_data_device_manager` 3 | Klavye/fare olayları, sürükle-bırak | Qt 6.9'da eski sürüm |
| `linux-dmabuf` | GPU ile çizilen arabellekler (VS Code, tarayıcılar) | Araştırılacak |
| `xdg-decoration` | Pencere çerçevesini compositor'ın çizmesi | Var (karar 0007) |
| `viewporter`, `fractional-scale` | Ölçekli ekranlar | Araştırılacak |
| `text-input` | Klavye düzeni, Türkçe karakterler, giriş yöntemleri | Araştırılacak |
| `primary-selection`, `xdg-activation`, `cursor-shape` | Seçili metni orta tuşla yapıştırma, pencere öne getirme, imleç | Araştırılacak |

Eksik bir protokol Qt'de yoksa Qt Wayland Compositor'ın C++ arayüzüyle kendimiz ekleriz.

## Test yöntemi

- **Birim testleri:**
  - Paket türünün tanınması (`.deb`, `.rpm`, AppImage, `.exe`)
  - `.desktop` dosyalarının okunması ve uygulama listesine çevrilmesi
  - Katman imzasının denetlenmesi (geçerli, değiştirilmiş, imzasız)
- **QEMU:** Katman yerel bir HTTP sunucusundan verilir, internet gerekmez. `ank kur` ile bir `.deb` kurulur,
  uygulama `qemu-test.py` ile açılıp ekran görüntüsü alınır.
- **UTM:** VS Code'un arm64 `.deb`'iyle gerçek kullanım: açılış, yazı yazma, Türkçe karakterler, GPU.

## Kapsam dışı

- Windows programları (Wine) ve x86_64 desteği: Faz 5
- x86 uygulamalarını ARM'de çalıştırmak (FEX-Emu, box64)
- Flatpak (ileride ayrı bir katman olarak eklenebilir)
- Uygulamaların yalıtılması (sandbox)
- Dosya yöneticisi (`.deb`'e çift tıklama dosya yöneticisi gelince tamamlanır)

## Riskler

| Risk | Önlem |
|---|---|
| Qt 6.9 compositor'ı VS Code gibi Electron uygulamalarının istediği protokolleri desteklemez | B adımı erkenden yapılır, önce küçük bir Wayland uygulamasıyla ölçülür. Gerekirse Qt sürümü yükseltilir. |
| Katmandaki Mesa ile ankaros'un GPU'su uyuşmaz (virgl) | C adımında `glxgears` ile denenir. Olmazsa yazılımsal çizime düşer, yavaş ama çalışır. |
| Katman indirmesi büyük ve yavaş | İlerleme gösterilir, indirme yarıda kalırsa kaldığı yerden devam eder |
| Katmanın kök dosya sistemi kaynağı (ör. images.linuxcontainers.org) değişir ya da kapanır | Kaynak yapılandırılabilir, imza ve özet denetlenir. Gerekirse kendi kopyamızı tutarız. |
| Electron uygulamaları Wayland'de sorunlu çalışır | X11 ile denenir (G adımı). VS Code'un Wayland desteği önce denenir. |
