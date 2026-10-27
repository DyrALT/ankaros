# Tasarım akışı: Figma'dan QML'e

Masaüstü Figma'da tasarlanır, `shell/` altında QML ile yazılır ve Mac önizlemesinde test edilir.
Son hali ankaros imajına paket olarak girer.

```
Figma (tasarım)  ──▶  Theme.qml + bileşenler  ──▶  Mac önizlemesi  ──▶  ankaros imajı
```

## 1. Figma dosyasının düzeni

Figma'daki yapı koddaki yapıyla eşleştiğinde aktarım çok kolaylaşır:

- **Değişkenler (Variables):** Renkler, boşluklar, köşe yarıçapları, yazı boyutları ve animasyon
  süreleri Figma değişkeni olarak tanımlanır. Her değişkenin `Theme.qml`'de aynı adla bir karşılığı vardır.
  Örnek: Figma'da `color/surface/panel` → QML'de `Theme.color.surfacePanel`.
- **Bileşenler (Components):** Panel, panel düğmesi, saat, başlatıcı kutucuğu gibi her parça bir Figma
  bileşeni olur. Her biri `shell/qml/Ankaros/Shell/` altında aynı adlı bir QML dosyasına karşılık gelir.
- **Çerçeve boyutu:** Masaüstü çerçeveleri **1440x900** boyutunda çizilir, önizleme penceresiyle aynı.
- **Açık/koyu mod:** Figma değişken modları (Light/Dark) olarak tanımlanır, `Theme.dark` ile değişir.

## 2. Tasarımı koda aktarmak

Figma'da bir ekran ya da bileşen hazır olduğunda bağlantısını (frame'e sağ tık → Copy link) Claude'a verin.
Claude, Figma bağlantısı üzerinden tasarımın ölçülerini, değişkenlerini ve görselini okuyup QML'e
aktarabilir. Bunun için Claude Code'da Figma bağlantısının açık ve yetkilendirilmiş olması gerekir.

Aktarma kuralları:

1. Önce değişkenler `Theme.qml`'e eklenir, sonra bileşenler yazılır.
2. Bileşenlerde değer doğrudan yazılmaz (`color: "#1e1e2e"` değil, `color: Theme.color.surfacePanel`).
3. Simgeler SVG olarak `shell/qml/Ankaros/Shell/icons/` altına aktarılır.
4. Animasyonlar Figma'daki Smart Animate süreleri ve eğrileri kullanılarak `Theme.motion` üzerinden yazılır.

## 3. Mac önizlemesinde test

```bash
./scripts/shell-preview.sh
```

Betik kabuğu derler (birkaç saniye) ve önizleme penceresini açar. Pencere Figma çerçevesiyle aynı
boyutta (1440x900) açılır, yeniden boyutlandırılabilir ve tam ekran yapılabilir. En küçük boyut
1024x720'dir. Figma ile karşılaştırmak için `--screenshot` ile alınan görüntüleri kullanın.

Önizleme penceresinde:

| Kısayol | Ne yapar |
|---|---|
| `Cmd+R` | QML'i yeniden yükler (C++ değişikliklerinde yeniden derleyin) |
| `Cmd+D` | Açık/koyu mod arasında geçiş yapar |
| `Ctrl+Cmd+F` ya da `F11` | Tam ekran |
| `Cmd+Q` | Kapatır |

Geliştirici seçenekleri (Figma ile karşılaştırma ya da ekran görüntüsü için):

```bash
APP=build/shell/preview/ankaros-shell-preview.app/Contents/MacOS/ankaros-shell-preview
$APP --dark --launcher --screenshot ekran.png   # koyu mod, başlatıcı açık, görüntüyü kaydet ve çık
```

| Seçenek | Ne yapar |
|---|---|
| `--dark` | Koyu modda açar |
| `--launcher` | Başlat paneli açık olarak başlar |
| `--screenshot <dosya>` | Masaüstünün görüntüsünü PNG olarak kaydeder ve çıkar. Pencere boyutundan bağımsız olarak 1440x900 çizilir |
| `--size <GxY>` | Ekran görüntüsünün boyutu, ör. `--size 1024x720` |

## Şu anki tasarım: B · Ankara Taşı

Figma dosyası: [ankaros: Masaüstü Tasarımı](https://www.figma.com/design/i51xbYLOkRyD5Qc5PhgFDv).
Dört varyasyon çizildi (A Kehribar, B Ankara Taşı, C Bozkır, D Anka), **B seçildi**: ekranın altında
yüzen tek bir çubuk (başlat düğmesi, arama, uygulamalar, durum simgeleri, saat) ve ortada açılan bir
başlat paneli (arama, sabitlenmiş uygulamalar, önerilenler, kullanıcı).

| Açık mod | Koyu mod, başlat paneli açık |
|---|---|
| ![Açık mod](gorseller/kabuk-acik.png) | ![Koyu mod, başlat paneli](gorseller/kabuk-koyu-baslatici.png) |

Figma'daki masaüstü ekranında görünen Dosyalar penceresi kabuğun parçası değildir. Pencereler Faz 1'de
compositor'dan gelecek.

Bileşenler (`shell/qml/Ankaros/Shell/`):

| Dosya | Bileşen | Figma karşılığı |
|---|---|---|
| `Theme.qml` | Tüm tasarım değerleri (tekil nesne) | `Theme` ve `Vurgu` değişken koleksiyonları |
| `IconData.js` | Simge yolları ve uygulama simgesi renkleri | Bileşenler sayfası |
| `Label.qml` | Inter yazı tipli metin | |
| `Icon.qml` | Çizgi simge (Qt Shapes ile çizilir) | `Icon/*` |
| `AppIcon.qml` | Degrade uygulama simgesi | `AppIcon/*` |
| `GlassPanel.qml` | Arkasını bulanıklaştıran buzlu cam yüzey | panel dolgusu + arka plan bulanıklığı |
| `HoverButton.qml` | Üzerine gelince arka planı beliren düğme | `color/surface/hover`, `pressed` |
| `Wallpaper.qml` | Renk geçişi ve yumuşak renk lekeleri | Duvar kağıdı |
| `Desktop.qml` | Masaüstünün kökü. Önizleme ve compositor bunu yükler | Ekran çerçevesi |
| `TaskBar.qml`, `TaskItem.qml`, `Clock.qml` | Alt çubuk, uygulama öğesi, saat | Alt çubuk |
| `StartMenu.qml`, `AppTile.qml`, `RecommendedItem.qml` | Başlat paneli ve öğeleri | Başlat paneli |

### Uygulama notları

- **Yazı tipi:** Inter 4.1 modülün içine gömülüdür (`fonts/`, SIL Open Font License). Böylece kabuk
  Mac'te ve ankaros'ta aynı görünür. Metinler için `Text` yerine `Label` kullanın.
- **Simgeler:** SVG dosyası yerine yol verisi olarak tutulur ve Qt Shapes ile GPU'da çizilir. Rengi
  temaya bağlanabilir ve Qt Svg modülüne ihtiyaç duyulmaz. Figma'ya yeni simge eklendiğinde yol
  verisi `IconData.js`'e eklenir (`<circle>`, `<rect>` gibi öğeler yola çevrilerek).
- **Buzlu cam:** `GlassPanel`, `backdrop` olarak verilen öğenin (duvar kağıdı) arkasında kalan
  bölümünü `MultiEffect` ile bulanıklaştırır. Faz 1'de backdrop, pencereleri de içeren sahne olacak.
- **Duvar kağıdı lekeleri:** Figma'daki bulanık elipsler yerine radyal degrade kullanılır. Görünüm
  aynıdır, çizim maliyeti çok düşüktür.

Yeni bir QML dosyası eklerken `shell/qml/Ankaros/Shell/CMakeLists.txt` içindeki `QML_FILES` listesine
de ekleyin.

## 4. ankaros'a almak

Faz 1'de `external/package/ankaros-shell` paketi `shell/` modülünü imaja derler. Mac'te çalışan QML
olduğu gibi kullanılır. Kabuk modülü Wayland'e özgü türleri import etmediği sürece (bkz.
[mimari.md](mimari.md)) iki ortam arasında kod farkı olmaz.
