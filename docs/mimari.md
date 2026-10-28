# Mimari

ankaros, Linux çekirdeğinin üzerine kurulmuş, kendi masaüstü kabuğu ve compositor'ı olan bir sistemdir.
Çekirdeği yazmıyoruz. Çekirdeğin üstünde kalan her şeyi biz tasarlıyoruz.

## Katmanlar

```
┌───────────────────────────────────────────────────────────┐
│ Uygulamalar (Wayland istemcileri)                         │  Faz 3+
├───────────────────────────────────────────────────────────┤
│ Masaüstü kabuğu  ·  QML (Ankaros.Shell modülü)            │  Faz 1
│ panel, başlatıcı, bildirimler, kilit ekranı               │
├───────────────────────────────────────────────────────────┤
│ Compositor  ·  C++ + Qt Wayland Compositor                │  Faz 1
│ pencere yönetimi, girdi, ekrana çizim                     │
├───────────────────────────────────────────────────────────┤
│ Grafik yığını  ·  Mesa (virgl), DRM/KMS, libinput         │  Faz 1
├───────────────────────────────────────────────────────────┤
│ Sistem servisleri  ·  init, ağ, oturum                    │  Faz 0: BusyBox init
│                                                           │  Faz 2: kendi init'imiz (C++)
├───────────────────────────────────────────────────────────┤
│ C kütüphanesi ve temel araçlar  ·  glibc, BusyBox, eudev  │  Faz 0
├───────────────────────────────────────────────────────────┤
│ Linux çekirdeği 6.18  ·  aarch64, virtio aygıtları        │  Faz 0
└───────────────────────────────────────────────────────────┘
```

## Kabuk ve compositor ilişkisi

Kabuk bir QML modülüdür (`Ankaros.Shell`), compositor'dan bağımsız yazılır. Bu sayede:

- **Mac'te geliştirme:** `shell/preview` uygulaması kabuğu sıradan bir pencerede gösterir. Tasarım ve
  animasyon işleri imaj derlemeden yapılır.
- **ankaros'ta çalışma:** Compositor aynı `Desktop` bileşenini ekranın kendisi olarak yükler, uygulama
  pencerelerini de onun içine yerleştirir. Taslak olarak:

```qml
WaylandCompositor {
    WaylandOutput {
        window: Window {
            Desktop {                       // Ankaros.Shell: Mac önizlemesindekiyle aynı bileşen
                anchors.fill: parent
                running: compositor.runningApps // penceresi açık uygulamalar compositor'dan gelir
                activeApp: compositor.focusedApp
                onAppLaunched: app => launcher.start(app)
            }
        }
    }
    XdgShell { onToplevelCreated: ... }
}
```

Gerçekte compositor (`compositor/Main.qml`) şöyle çalışır:

- `WaylandCompositor` `wayland-0` soketinde bir Wayland sunucusu açar, tam ekran bir `Window` içinde
  `Desktop`'u gösterir.
- Bir uygulama pencere açtığında (`XdgShell.onToplevelCreated`) `Desktop.windowLayer` içine bir
  `WaylandWindow` oluşturulur. `WaylandWindow`, kabuktaki `WindowChrome` çerçevesinin içine uygulamanın
  yüzeyini (`ShellSurfaceItem`) koyar.
- Pencerenin uygulama adı Wayland `app_id`'sinden bulunur. Alt çubuktaki "çalışıyor" ve "etkin"
  göstergeleri buradan gelir.
- Alt çubukta ya da başlat panelinde bir uygulamaya tıklanınca: penceresi yoksa `AppLauncher` (C++)
  onu `WAYLAND_DISPLAY=wayland-0` ile başlatır, odaktaysa küçültür, değilse öne getirir.
- Kendi kenarlığını çizen uygulamalarda (xdg-decoration desteklemeyenler) ankaros çerçevesi çizilmez
  (karar 0007).
- Pencerelerin üst üste sırası `Desktop.nextZ()` ile verilir. Kabuğun kendi pencereleri (Ayarlar) ve
  uygulama pencereleri aynı sıralamayı paylaşır.

Kabuk modülü ankaros'ta paylaşılan bir QML modülü olarak Qt'nin QML klasörüne kurulur, Mac
önizlemesinde ise uygulamaya statik olarak gömülür (`ANKAROS_SHELL_SHARED`).

Kural şu: **kabuk modülü Wayland'e özgü hiçbir türü doğrudan import etmez.** Uygulama listesi, çalışan
uygulamalar ve öneriler gibi veriler dışarıdan property olarak verilir, kullanıcının istekleri
(`appLaunched` gibi) sinyal olarak dışarı çıkar. Mac önizlemesi bu verileri sahte (mock) verilerle doldurur.

## Açılış sırası

Faz 0'daki durum:

1. QEMU, çekirdeği (`Image`) ve kök dosya sistemini (`rootfs.ext4`, virtio-blk) yükler.
2. Çekirdek `/dev/vda`'yı kök olarak bağlar, `/sbin/init`'i (BusyBox) başlatır.
3. `/etc/init.d/` betikleri: eudev, ağ (DHCP, `eth0`), rastgele sayı tohumu.
4. Seri konsolda (`ttyAMA0`) giriş istemi.

Faz 1'den itibaren:

5. `/etc/init.d/S90ankaros-desktop` compositor'ı root olarak arka planda başlatır. Ortam değişkenleri
   `/etc/profile.d/ankaros-grafik.sh`'den gelir, çıktı `/var/log/ankaros-compositor.log`'a yazılır.
6. Ekranda masaüstü açılır. Seri konsoldaki giriş istemi kurtarma ve geliştirme için durur.

Kullanıcı hesabı ve giriş ekranı Faz 2'de.

## Repo yapısı

| Yol | İçerik |
|---|---|
| `buildroot/` | Buildroot, git submodule, LTS sürümüne sabitli. **İçinde değişiklik yapılmaz.** |
| `external/` | Buildroot'a dışarıdan eklenen ankaros ağacı (BR2_EXTERNAL) |
| `external/configs/` | `ankaros_qemu_aarch64_defconfig`: sistemin tüm yapılandırması |
| `external/board/qemu-aarch64/` | Karta özel dosyalar: rootfs overlay, post-build betiği |
| `external/package/` | ankaros'un kendi paketleri: `ankaros-shell`, `ankaros-compositor` |
| `shell/` | Masaüstü kabuğu: QML modülü ve Mac önizleme uygulaması |
| `compositor/` | Wayland compositor: `Main.qml` (sunucu ve pencere yönetimi), `WaylandWindow.qml`, `AppLauncher` |
| `scripts/` | Derleme, çalıştırma ve ilerleme betikleri |
| `docs/` | Belgeler |

Buildroot'ta bir şeyi değiştirmek gerekirse ya `external/` içinde bir ayarla ya da
`external/patches/` altında bir yamayla yapılır, submodule'ün içi elle düzenlenmez.

## Derleme ortamı

```
 macOS (Apple Silicon)                       OrbStack: ankaros-build (Debian arm64)
 ┌────────────────────────────┐              ┌──────────────────────────────────┐
 │ ~/Desktop/ankaros  (repo)  │ ── paylaşım ─▶│ ~/Desktop/ankaros  (aynı yol)    │
 │                            │              │ ~/ankaros-output   (Buildroot)   │
 │ scripts/build.sh ──────────┼── orb ──────▶│ make O=~/ankaros-output          │
 │                            │              │ ~/ankaros-dl, ~/.buildroot-ccache│
 │ scripts/run-qemu.sh        │◀── images ───┤ ~/ankaros-output/images          │
 │ QEMU (-accel hvf)          │              └──────────────────────────────────┘
 │ shell/preview (Qt, yerel)  │
 └────────────────────────────┘
```

Neden bu düzen seçildi: [kararlar/0003-aarch64-orbstack-qemu.md](kararlar/0003-aarch64-orbstack-qemu.md)
