# 0019: X11 uygulamaları için xwayland-satellite

- **Durum:** Kabul edildi
- **Tarih:** 2026-11-09

## Bağlam

Debian katmanından kurulan uygulamaların bir kısmı yalnızca X11 biliyor: bazı Electron uygulamaları, Java
uygulamaları, eski araçlar, ileride Wine (Faz 6). Bunlar Wayland'de Xwayland ile çalışır. Xwayland'in X11
pencerelerini masaüstünde ayrı pencereler olarak göstermesi (rootless) için birinin X11 pencere yöneticisi (XWM)
olması gerekir: pencereleri yüzeylerle eşleştirmek, ICCCM/EWMH özelliklerini okumak, panoyu ve sürükle-bırakı
iki dünya arasında taşımak. wlroots, Weston ve KWin bunu compositor'ın içinde yapar. Qt Wayland Compositor'da
([karar 0006](0006-qt-wayland-compositor-ve-eglfs.md)) böyle bir destek yok.

## Değerlendirilen seçenekler

- **Kendi XWM'imiz (C++, xcb, compositor'ın içinde):** En sıkı bütünleşme. Ama binlerce satırlık, X11'in köşe
  durumlarıyla dolu bir iş. Pano ve sürükle-bırak köprüsü ayrıca yazılmalı.
- **Pencereli (rootful) Xwayland:** Her X11 uygulaması tek bir büyük Xwayland penceresinin içinde açılır. Hızlı
  biter ama pencere yönetimi yok: Alt+Tab'da, alt çubukta ve yaslamada X11 uygulamaları görünmez.
- **xwayland-satellite:** Xwayland ile compositor arasında duran, X11 pencerelerini sıradan `xdg_toplevel`
  pencerelerine çeviren hazır bir araç (Rust, MPL-2.0). niri bu yolla çalışıyor. Compositor'dan yalnızca
  `xdg_wm_base` ve `wp_viewporter` ister, ikisi de var. Pano ve birincil seçimi kendisi taşır.

## Karar

xwayland-satellite kullanılır. Xwayland ve satellite Buildroot'la imaja girer, katmana değil: X11 uygulaması
hangi katmandan gelirse gelsin aynı X sunucusunu kullanır.

- X sunucusu ihtiyaç olunca başlar. `ankaros-x11` masaüstüyle birlikte açılır, X soketini
  (`/tmp/.X11-unix/X0`) dinler ve ilk uygulama bağlanınca satellite'i soketle (`-listenfd`) başlatır. X11
  uygulaması açılmayan oturumda ne Xwayland ne satellite çalışır.
- Soket yalnızca dosya sisteminde açılır (0600), soyut ad alanında açılmaz. Soyut soketlerin izni yok, X
  sunucusu da kimlik doğrulama istemediği için makinedeki herkes bağlanıp klavyeyi okuyabilirdi.
- satellite'in bir bağımlılığı bindgen ile derlenir, bindgen libclang ister. Derleme makinesinin clang'ı yerine
  Buildroot'un `host-clang`'ı kullanılır.

## Sonuçlar

- X11 pencereleri compositor'a sıradan pencereler olarak gelir: başlık çubuğunu biz çizeriz, Alt+Tab, alt
  çubuk, yaslama ve tam ekran çalışır.
- satellite'in bilmediği X11 davranışları (bazı Java uygulamaları, karma DPI) onun sınırlarıdır. Sürümü
  Buildroot paketinde sabittir, güncellemek bizim işimiz.
- `host-clang` LLVM'i derler: önbelleksiz imaj derlemesi uzar (CI'da Buildroot çıktısı önbellekte, yalnızca
  `external/` ya da Buildroot değişince baştan derlenir).
- İmaja Xwayland, X11 kütüphaneleri ve xkbcomp girer (birkaç MB). `BR2_PACKAGE_XORG7` açık olduğu için bazı
  paketler (Mesa gibi) X11 desteğiyle derlenebilir.
- Aynı anda tek oturum olduğu için ekran numarası sabit (`:0`).
