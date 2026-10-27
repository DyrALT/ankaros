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
boyutta olduğu için ekran görüntüsünü Figma'daki tasarımla yan yana karşılaştırabilirsiniz.

Önizleme penceresinde:

| Kısayol | Ne yapar |
|---|---|
| `Cmd+R` | QML'i yeniden yükler (C++ değişikliklerinde yeniden derleyin) |
| `Cmd+D` | Açık/koyu mod arasında geçiş yapar |
| `Cmd+Q` | Kapatır |

Geliştirici seçenekleri (Figma ile karşılaştırma ya da ekran görüntüsü için):

```bash
APP=build/shell/preview/ankaros-shell-preview.app/Contents/MacOS/ankaros-shell-preview
$APP --dark --launcher --screenshot ekran.png   # koyu mod, başlatıcı açık, görüntüyü kaydet ve çık
```

| Seçenek | Ne yapar |
|---|---|
| `--dark` | Koyu modda açar |
| `--launcher` | Başlatıcı açık olarak başlar |
| `--screenshot <dosya>` | Masaüstünün görüntüsünü PNG olarak kaydeder ve çıkar |

## Şu anki prototip

Figma tasarımı gelene kadar yer tutucu bir tasarım var: duvar kağıdı, altta yüzen bir panel (başlatıcı
düğmesi ve saat) ve tam ekran uygulama başlatıcı (arama ile).

| Açık mod | Koyu mod, başlatıcı açık |
|---|---|
| ![Açık mod](gorseller/kabuk-acik.png) | ![Koyu mod, başlatıcı](gorseller/kabuk-koyu-baslatici.png) |

Bileşenler (`shell/qml/Ankaros/Shell/`):

| Dosya | Bileşen |
|---|---|
| `Theme.qml` | Tüm tasarım değerleri (tekil nesne). Figma değişkenlerinin karşılığı |
| `Desktop.qml` | Masaüstünün kökü. Önizleme ve compositor bunu yükler |
| `Wallpaper.qml` | Duvar kağıdı |
| `Panel.qml`, `PanelButton.qml` | Alttaki panel ve düğmeleri |
| `LauncherIcon.qml` | Başlatıcı simgesi (geçici) |
| `Clock.qml` | Saat ve tarih |
| `Launcher.qml`, `AppTile.qml` | Uygulama başlatıcı ve uygulama kutucuğu |

Yeni bir QML dosyası eklerken `shell/qml/Ankaros/Shell/CMakeLists.txt` içindeki `QML_FILES` listesine
de ekleyin.

## 4. ankaros'a almak

Faz 1'de `external/package/ankaros-shell` paketi `shell/` modülünü imaja derler. Mac'te çalışan QML
olduğu gibi kullanılır. Kabuk modülü Wayland'e özgü türleri import etmediği sürece (bkz.
[mimari.md](mimari.md)) iki ortam arasında kod farkı olmaz.
