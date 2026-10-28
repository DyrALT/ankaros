# Pencere yönetimi

Uygulama pencerelerini compositor yönetir (`compositor/`). Pencere çerçevesi (gölge, başlık çubuğu,
boyutlandırma tutamakları) `Ankaros.Shell` modülündeki `WindowChrome`'dur. Wayland'e bağlı değildir, kabuk
önizlemesinde de kullanılır. Tasarım: Figma'da "Faz 3 · Masaüstü cilası" sayfası.

## Davranışlar

| İş | Nasıl |
|---|---|
| Taşıma | Başlık çubuğundan sürükleyerek. Başlık çubuğu ekranın üstünden dışarı çıkmaz. |
| Boyutlandırma | Kenarlardan ve köşelerden sürükleyerek. Tutamaklar çerçevenin hemen dışındadır (6 px), uygulamanın içine taşmaz. Uygulamanın en küçük ve en büyük boyutu korunur. |
| Yaslama | Pencere ekranın sol ya da sağ kenarına sürüklenince yarım ekrana, üst kenara sürüklenince tam ekrana yaslanır. Bırakmadan önce yarı saydam bir önizleme yerini gösterir. |
| Eski boyuta dönme | Büyütülmüş ya da yaslanmış pencere sürüklenince eski boyutuna döner, fare pencerenin aynı oranındaki yerinde kalır. Başlık çubuğuna çift tıklamak da büyütür ya da eski boyutuna döndürür. |
| Pencere geçişi | Alt+Tab: açık pencerelerin canlı küçük görüntüleri, son kullanılandan başlayarak. Alt bırakılınca seçilen pencere öne gelir. Küçültülmüş pencerenin yerinde uygulamanın simgesi görünür. |
| Animasyonlar | Açılma ve kapanma (soluklaşma ve hafif büyüme), küçültme (alt çubuktaki simgesine doğru uçar) |

## Kısayollar

| Kısayol | İş |
|---|---|
| Süper | Başlat panelini aç ve kapat (tek başına basılıp bırakılınca) |
| Süper+D | Bütün pencereleri küçült, ikinci basışta geri getir |
| Süper+← / → | Etkin pencereyi sola ya da sağa yasla |
| Süper+↑ | Büyüt |
| Süper+↓ | Büyütülmüş ya da yaslanmışsa eski boyutuna döndür, değilse küçült |
| Alt+Tab, Alt+Shift+Tab | Pencereler arasında geçiş, Esc vazgeçer |
| Alt+F4 | Etkin pencereyi kapat |
| Süper+L | Ekranı kilitle |

Kısayollar giriş ve kilit ekranında çalışmaz.

## Yapı

| Dosya | İş |
|---|---|
| `compositor/Main.qml` | Pencere listesi, son kullanılan sırası (`focusHistory`), Alt+Tab, Süper+D ve kısayollar |
| `compositor/WaylandWindow.qml` | Tek pencere: yerleştirme, büyütme, yaslama, boyutlandırma, animasyonlar |
| `compositor/globalkeys.{h,cpp}` | Uygulamaya gitmeden önce yakalanan tuşlar: tek başına Süper ve Alt+Tab. QML `Shortcut`'ı tuşun bırakılmasını göremediği için C++ olay süzgeci. Alt+Tab'daki Tab uygulamaya iletilmez. |
| `shell/qml/Ankaros/Shell/WindowChrome.qml` | Çerçeve, taşıma ve boyutlandırma tutamakları. Pencereyi kendisi değiştirmez, sinyallerle bildirir. |
| `shell/qml/Ankaros/Shell/WindowSwitcher.qml` | Alt+Tab görünümü |
| `shell/qml/Ankaros/Shell/Desktop.qml` | Yaslama önizlemesi ve pencere geçişinin masaüstündeki yeri |

### Wayland tarafı

- **Boyutlandırma:** Fare hareket ettikçe uygulamaya `resizing` durumuyla yeni boyut gönderilir
  (`XdgToplevel.sendResizing`). Uygulama yeni boyutta bir kare gönderince çerçeve büyür. Soldan ya da
  üstten boyutlandırırken karşı kenarın yerinde kalması için pencerenin konumu bu karede düzeltilir.
- **Yaslama:** xdg-shell'in `tiled` durumları Qt 6.9'un compositor API'sinde yok. Yaslanmış pencereye
  yalnızca yeni boyutu gönderilir (`sendUnmaximized(boyut)`). Uygulamalar kendilerini normal pencere sanar,
  bu yüzden köşelerini yuvarlamaya devam edebilirler.
- **Kapanma animasyonu:** Uygulama yüzeyini yok edince son karesi ekranda tutulur (`bufferLocked`),
  pencere soluklaşarak kaybolur.

### Örtülen pencerenin tıklama alması

Qt Quick, bir tıklamayı üstteki öğe alsa bile alttaki öğelerin sürükleme ve dokunma tutucularına
(`DragHandler`, `TapHandler`) da iletir. Önlem alınmazsa terminalde metin seçerken arkasında kalan bir
pencerenin başlık çubuğu tutulup taşınabiliyordu. Bu yüzden başlık çubuğunun `containmentMask`'ı
compositor'a "bu noktada en üstteki pencere ben miyim" diye sorar (`WindowChrome.isTopAt`).

## Test

```bash
# Kabuk önizlemesinde Alt+Tab görünümü
./scripts/shell-preview.sh --switcher
```

QEMU'da (`qemu-test.py`) tuş basılı tutma (`keydown alt`, `keyup alt`), bırakmadan sürükleme (`hold`, `up`)
ve imleç taşıma (`move`) eylemleriyle denenir:

```bash
./scripts/qemu-test.py --expect-desktop --wait 120 --actions \
  "click 326 860; sleep 8; key meta_l; sleep 3; click 771 513; sleep 10; \
   keydown alt; key tab; sleep 25; shot alt-tab.png; keyup alt; sleep 10; \
   hold 600 80 2 400; sleep 25; shot yaslama.png; up; sleep 15" --screenshot son.png
```

QEMU'nun yazılımsal GPU'sunda (softpipe) bulanıklık efektli bir ekran karesi bir buçuk saniye kadar sürer.
Animasyonlar kare başına ilerlediği için çok yavaş görünür, ekran görüntüsünden önce uzun beklemek gerekir.
Gerçek hız UTM'de (virgl) denenir.
