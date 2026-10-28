# ankaros'u UTM'de çalıştırmak

`run-qemu.sh --gui` ve CI testi yazılımsal çizim (softpipe) kullanır: masaüstü açılır ama
animasyonlar yavaştır. UTM'de sanal GPU'nun 3B hızlandırması açılabilir: ankaros'taki Mesa'nın virgl
sürücüsü çizim komutlarını UTM'ye gönderir, UTM de bunları Mac'in GPU'sunda Metal ile çizer (ANGLE).
Masaüstünün gerçek hızını görmek için UTM kullanılır.

UTM ücretsizdir: <https://mac.getutm.app>. UTM 4.7.5 ile denendi.

## Kurulum (betikle)

İmaj derlendikten sonra:

```bash
./scripts/utm-olustur.sh
```

Betik UTM'de `ankaros` adında bir sanal makine oluşturur. UTM'de onu başlatınca birkaç saniye içinde
masaüstü gelir. Başka bir ad için: `./scripts/utm-olustur.sh deneme`.

- İlk çalıştırmada macOS, terminalin UTM'yi denetlemesi için izin ister. İzin verilmezse betik takılır.
- UTM'nin betik arayüzü kernel ile açılışı ayarlayamadığı için betik UTM'yi bir kez kapatıp açar.
  Çalışan bir sanal makine varken betik durur, hiçbir şeyi kapatmaz.
- Kök dosya sistemi sanal makinenin klasörüne kopyalanır. Sonradan yeni bir imaj derlemek sanal
  makineyi değiştirmez. Yeni imajı denemek için sanal makineyi UTM'den kapatıp diskini ve kernel'ini
  güncelle:

  ```bash
  ./scripts/utm-olustur.sh --guncelle            # "ankaros" sanal makinesi
  ./scripts/utm-olustur.sh --guncelle deneme     # başka bir ad
  ```

  Sanal makinenin içinde yapılan değişiklikler (değiştirilen şifre, ağ ayarı) bu işlemde silinir.

## Sanal makinenin ayarları

Betiğin kurduğu ayarlar. Elle kurulum yapılırsa ya da bir şey ters giderse bunlarla karşılaştır:

| Ayar | Değer | Neden |
|---|---|---|
| Arka uç | QEMU (Apple Virtualization değil) | GPU hızlandırması yalnızca QEMU arka ucunda var |
| Mimari, makine | aarch64, `virt`, hipervizör açık, UEFI kapalı | Kernel doğrudan açılır, önyükleyici yok |
| Bellek, işlemci | 2048 MB, 4 çekirdek | |
| Display | `virtio-gpu-gl-pci` | 3B hızlandırmalı sanal GPU |
| Drives | Linux Kernel: `Image`. Disk: kök dosya sistemi, arayüz VirtIO | Disk ankaros'ta `/dev/vda` olur |
| QEMU › Arguments | `-append "rootwait root=/dev/vda console=ttyAMA0"`, `-device virtio-keyboard-pci`, `-device virtio-tablet-pci` | Çekirdekte USB girdi sürücüsü yok, klavye ve fare virtio aygıtlarıdır |

UTM, Arguments'taki her satırı boşluklardan böler. Birden çok kelimelik değerler tırnak içinde
yazılmalıdır, yoksa QEMU `root=/dev/vda`'yı dosya adı sanıp açılmaz.

## GPU hızlandırması çalışıyor mu?

Seri konsolda (UTM'de sanal makinenin Serial penceresi) ya da Terminal'de:

```bash
dmesg | grep virgl                            # "[drm] features: +virgl ..." görünmeli
echo 'export QSG_INFO=1' > /etc/profile.d/qsg-info.sh
ankarosctl restart ankaros-session
grep RENDERER /var/log/ankaros-compositor.log
```

Hızlandırma açıksa son komut şunu gösterir:

```text
OpenGL VENDOR: Mesa RENDERER: virgl (ANGLE (Apple, ANGLE Metal Renderer: Apple M2 Pro, ...) VERSION: OpenGL ES 3.0 Mesa 26.0.1
```

`softpipe` ya da `llvmpipe` görünüyorsa sanal makine 2B GPU ile açılmıştır, Display ayarını kontrol et.

## Sorunlar

| Belirti | Neden ve çözüm |
|---|---|
| `Could not open 'root=/dev/vda'` | `-append` değeri tırnak içinde değil |
| `VFS: Unable to mount root fs` | Kök dosya sistemi diski VirtIO arayüzünde değil |
| Masaüstü geliyor, fare ve klavye çalışmıyor | QEMU argümanlarındaki virtio girdi aygıtları eksik |
| `Failed to initialize EGL display ... EGL_WL_bind_wayland_display` | Zararsız. Uygulamalar ekran belleğini GPU üzerinden değil, paylaşılan bellekle gönderir. |
| Betik "Sanal makine oluşturuluyor"da takılıyor | macOS'un otomasyon izni penceresi cevap bekliyor |
