# Faz 1 planı: grafik masaüstü (`v0.2.0`)

**Hedef:** ankaros açıldığında giriş istemi yerine Ankara Taşı masaüstü gelir. Alt çubuktan ya da başlat
panelinden bir uygulama açılabilir, pencereleri taşınabilir, odaklanabilir, küçültülüp büyütülebilir ve
kapatılabilir. Kullanıcı pencere düğmelerinin konumunu ve stilini ayarlardan değiştirebilir.

## Mimari

```
Linux (DRM/KMS, virtio-gpu)
└── ankaros-compositor   C++ + QML, Qt Wayland Compositor, eglfs (X yok, doğrudan ekrana çizer)
    ├── Desktop          Ankaros.Shell: duvar kağıdı, alt çubuk, başlat paneli, ayarlar
    │   └── pencere katmanı   uygulama pencereleri ve ankaros başlık çubukları
    └── Wayland sunucusu      foot, Qt uygulamaları vb. buraya bağlanır
```

İlgili kararlar:

- [0006: Compositor için Qt Wayland Compositor ve eglfs](kararlar/0006-qt-wayland-compositor-ve-eglfs.md)
- [0007: Pencere kenarlıklarını compositor çizer, kullanıcı özelleştirebilir](kararlar/0007-sunucu-tarafi-pencere-kenarliklari.md)
- [0008: Kullanıcı ayarları QtCore Settings ile ~/.config/ankaros altında](kararlar/0008-kullanici-ayarlari.md)

## Kullanılacak Buildroot paketleri

Buildroot 2026.02'de hepsi hazır paket olarak var:

| Paket | Neden |
|---|---|
| qt6base (gui, opengl es2, eglfs, fontconfig, png) | Qt'nin temeli, eglfs ile doğrudan ekrana çizim |
| qt6declarative, qt6shadertools | QML, Qt Quick, Shapes, Effects |
| qt6wayland (compositor) | Qt Wayland Compositor |
| mesa3d (gbm, egl, gles; virgl ve softpipe sürücüleri) | Sanal GPU (virgl), GPU olmayan ortamlar için yazılımsal çizim (softpipe) |
| libinput, libxkbcommon, xkeyboard-config | Klavye ve fare |
| foot | Küçük bir Wayland terminali, ilk test uygulaması |

**Qt sürümü:** Buildroot'ta Qt 6.9 var, Mac'te Homebrew ile Qt 6.12 kullanıyoruz. Kabuğun 6.9'da olmayan
bir özelliği kullanmaması için CI'daki kabuk işi 6.9'a sabitlenir.

## Adımlar

| PR | İçerik |
|---|---|
| **A** `build(buildroot)` | Yukarıdaki paketler, çekirdekte DRM ve virtio girdi aygıtları, Inter yazı tipi. `run-qemu.sh --gui` ile grafik modu. Basit bir QML test uygulamasının ekranda açılması. İlk derleme birkaç saat sürer. |
| **B** `feat(compositor)` | `compositor/`: Desktop tam ekran, Wayland sunucusu (xdg-shell, xdg-decoration), foot penceresinin açılması, ankaros başlık çubuğu, taşıma, odak, kapatma |
| **C** `feat(shell)` | Pencere katmanı. Alt çubuğun "çalışıyor" ve "etkin" göstergelerinin gerçek pencerelerden gelmesi. Başlat panelinden uygulama açma (`/etc/ankaros/apps.json`). Küçültme ve büyütme. **Görünüm ayarları:** pencere düğmelerinin konumu ve stili, açık/koyu mod. |
| **D** `build` | `ankaros-shell` ve `ankaros-compositor` Buildroot paketleri, açılışta compositor'ın otomatik başlaması |
| **E** `test` + `docs` | QEMU'da otomatik açılış testi (ekran görüntüsüyle), UTM kurulum rehberi, `v0.2.0` |

A'nın uzun derlemesi sürerken B'nin kodu yazılabilir. B, A bitmeden denenemez.

## Pencere düğmelerinin özelleştirilmesi

Başlık çubuğunu compositor çizdiği için ayar bütün uygulamalara aynı anda uygulanır.

| Ayar | Seçenekler | Varsayılan |
|---|---|---|
| Düğme konumu | Sağ, sol | Sağ |
| Düğme stili | Çizgi simgeler (Figma'daki B tasarımı), renkli daireler (üzerine gelince simge belirir) | Çizgi simgeler |
| Görünüm | Açık, koyu | Açık |

- Ayarlar, başlat panelindeki Ayarlar düğmesinden açılan **Görünüm** bölümünde değiştirilir.
- Değişiklik açık pencerelere animasyonla, anında yansır.
- Ayarlar `~/.config/ankaros/shell.conf` dosyasında saklanır, yeniden başlatmada korunur.
- Mac önizlemesinde de aynı ayarlar çalışır, tasarım orada denenebilir.
- Tasarım önce Figma'da yapılır: başlık çubuğu bileşeni (konum × stil varyantları) ve Görünüm ekranı.

## Test yöntemi

- **UTM (günlük kullanım):** virtio-gpu-gl ile GPU hızlandırmalı, virgl sürücüsü. Gerçek hızı görmek için.
- **QEMU ve CI (otomatik):** virtio-gpu (2D) ve softpipe ile yazılımsal çizim. İmaj açılır, QEMU'nun
  ekran görüntüsü komutuyla (`screendump`) masaüstünün göründüğü doğrulanır.
- **Mac önizlemesi:** Kabuk, ayarlar ekranı ve başlık çubuğu tasarımı compositor olmadan da denenebilir.

## Kapsam dışı (Faz 2 ve sonrası)

- Kullanıcı hesabı ve giriş ekranı. Faz 1'de sistem doğrudan root olarak masaüstüne açılır.
- Gerçek Wi-Fi, pil ve ses bilgisi. Durum simgeleri görsel olarak kalır.
- Pencereleri ekranın yarısına yerleştirme (snap), çalışma alanları, bildirimler
- Ayrı bir Ayarlar uygulaması. Faz 1'deki Görünüm bölümü kabuğun içindedir.
- Kendi init sistemimiz

## Riskler

| Risk | Önlem |
|---|---|
| Qt 6.9 (imaj) ile 6.12 (Mac) arasındaki farklar | CI'daki kabuk işi 6.9'a sabitlenir |
| İlk Qt derlemesi uzun sürer (birkaç saat, ~25 GB disk) | ccache ve indirme önbelleği. CI'da imaj işinin süre sınırı yükseltilir. |
| softpipe ile Qt Quick çok yavaş çalışabilir | softpipe yalnızca otomatik testte kullanılır, günlük kullanım UTM ve virgl ile |
| eglfs'te girdi ve odak sorunları | B'de önce tek pencereyle (foot) başlanır, adım adım genişletilir |
