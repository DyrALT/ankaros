# X11 uygulamaları (XWayland)

ankaros'un masaüstü Wayland'dir. Yalnızca X11 bilen uygulamalar (bazı Electron ve Java uygulamaları, eski
araçlar) Xwayland ile çalışır. Compositor X11 pencere yöneticisi değildir: araya xwayland-satellite girer ve her
X11 penceresini sıradan bir Wayland penceresine çevirir ([karar 0019](kararlar/0019-xwayland-satellite.md)).

```
X11 uygulaması (Debian katmanı)
   │  DISPLAY=:0, /tmp/.X11-unix/X0
   ▼
Xwayland  ◄──  xwayland-satellite  ──►  ankaros-compositor (xdg_toplevel)
   ▲
   └── ankaros-x11: soketi açar, ilk bağlantıda satellite'i başlatır
```

## Nasıl başlar

- **Açılışta:** init `/tmp/.X11-unix` klasörünü root olarak, yapışkan bitle (1777) oluşturur.
- **Masaüstü açılınca:** `masaustu-baslat` `DISPLAY=:0`'ı ayarlar ve `ankaros-x11`'i kullanıcı olarak başlatır.
  `ankaros-x11` kilit dosyasını (`/tmp/.X0-lock`) ve soketi (`/tmp/.X11-unix/X0`, 0600) açar, sonra bekler.
- **İlk X11 uygulaması bağlanınca:** `ankaros-x11` `xwayland-satellite :0 -listenfd <soket>`'i başlatır, satellite
  de Xwayland'i. Bekleyen bağlantı kaybolmaz, uygulama açılır.
- **satellite çıkarsa:** `ankaros-x11` soketi yeniden bekler, sonraki X11 uygulaması onu tekrar başlatır. Üst üste
  beş kez 5 saniyeden kısa sürede çıkarsa vazgeçer, günlüğe yazar.
- **Oturum kapanınca:** `masaustu-baslat` `ankaros-x11`'i durdurur. O da satellite'i durdurur, soketi ve kilidi
  siler.

X11 uygulaması açılmayan oturumda Xwayland ve satellite hiç çalışmaz, bellek ve açılış süresi etkilenmez.

Debian katmanı `/tmp`'yi paylaşır ([katmanlar.md](katmanlar.md)). Katmanda başlatılan uygulamalar `DISPLAY`'i
compositor'dan devralır ve aynı soketle bağlanır.

## Güvenlik

- Soket yalnızca dosya sisteminde açılır ve yalnızca kullanıcıya açıktır (0600). Soyut ad alanındaki soket
  (`@/tmp/.X11-unix/X0`) açılmaz: onun izni yoktur, X sunucusu da kimlik doğrulaması (Xauthority) istemediği için
  başka bir kullanıcı bağlanıp ekranı ve klavyeyi okuyabilirdi.
- X11'in kendisinde uygulamalar arası yalıtım yoktur: aynı X sunucusuna bağlanan X11 uygulamaları birbirinin
  penceresini ve tuşlarını görebilir. Wayland uygulamaları bundan etkilenmez.

## Yamalar

- **Qt (`external/patches/qt6wayland/0002-xdg-wm-base-2.patch`):** Qt 6.9 `xdg_wm_base` 1 sunuyor, satellite en az 2
  ister. Sürüm 2 yalnızca yeni pencere durumları ekler.
- **Qt (`0003-xdg-output-3-done.patch`):** `zxdg_output_v1` 3'te ekranın mantıksal boyutu `wl_output.done` ile
  uygulanır, Qt bağlanma anında yalnızca `zxdg_output_v1.done` gönderiyordu. Xwayland ekranı 0x0 sanıyor, fare
  imleci (0,0)'a sabitleniyordu: X11 pencereleri açılıyor, klavye çalışıyor ama tıklanamıyordu.
- **satellite (`external/patches/xwayland-satellite/`):** Ekranın adı `wl_output` 4'ün `name` olayıyla gelir, biz
  `wl_output` 2 sunuyoruz. satellite ad gelmeden pencere ekrana girince çöküyordu, yama adı isteğe bağlı yapar.

## Sınırlar

- **Ekran numarası sabit (`:0`):** Aynı anda tek oturum var. Kilit başka birinin süreciyse X11 uygulamaları açılmaz,
  günlükte yazar.
- **Pencere menüleri:** `xdg_wm_base` 2'de açılır menülerin yeniden konumlanması yok, bazı X11 menüleri yanlış
  yerde açılabilir.
- **satellite'in sınırları:** Bazı Java uygulamaları boş pencere gösterebilir (`_JAVA_AWT_WM_NONREPARENTING=1`
  çözer). Ekran ölçeği X11 uygulamalarına tek bir DPI olarak gider.
- **Uygulama simgesi ve adı:** X11 penceresinin `app_id`'si satellite'in `WM_CLASS`'tan verdiğidir. Alt çubuktaki
  eşleşme `.desktop`'taki `StartupWMClass`'ı okumaz, bazı X11 uygulamaları genel simgeyle görünebilir.

## Deneme

QEMU'da `xeyes` (Buildroot'un `xapp_xeyes` paketi, imajda yok, yerelde `make xapp_xeyes all` ile eklendi)
ankaros başlık çubuğuyla ayrı bir pencerede açıldı ve alt çubukta göründü. Xwayland yaklaşık 9 MB bellek kullandı.
Başka bir kullanıcı (`nobody`) X ekranına bağlanamadı. `xev` fare hareketlerini ve tıklamaları doğru
koordinatlarla aldı, X ekranı 1x'te 1440x900, 2x'te (Retina) 2880x1800.

Debian katmanında küçük bir X11 uygulaması:

```sh
ank katman calistir debian -- apt install -y x11-apps
ank katman calistir debian -- xeyes
```

`xeyes` ankaros başlık çubuğuyla ayrı bir pencerede açılmalı, Alt+Tab'da görünmeli. Günlükte (oturum günlüğü)
`ankaros-x11` ve satellite satırları görünür.
