# ankaros'u UTM'de çalıştırmak

> **Durum:** Bu rehber henüz baştan sona denenmedi. İlk deneyen, eksik ya da yanlış adımları
> bu dosyada düzeltsin.

`run-qemu.sh --gui` ve CI testi yazılımsal çizim (softpipe) kullanır: masaüstü açılır ama
animasyonlar yavaştır. UTM'de sanal GPU'nun 3B hızlandırması (virtio-gpu-gl ve Mesa'nın virgl
sürücüsü) açılabilir. Masaüstünün gerçek hızını görmek için UTM kullanılır.

UTM ücretsizdir: <https://mac.getutm.app> (App Store sürümüyle aynı, ücretsiz olanı yeterli).

## 1. İmajı hazırla

UTM, OrbStack'in paylaşılan klasöründen dosya seçemeyebilir. Bu yüzden imajı önce Mac'te sıradan bir
klasöre kopyala:

```bash
mkdir -p ~/ankaros-utm
cp ~/OrbStack/ankaros-build/home/$USER/ankaros-output/images/Image ~/ankaros-utm/
cp ~/OrbStack/ankaros-build/home/$USER/ankaros-output/images/rootfs.ext4 ~/ankaros-utm/rootfs.img
```

CI'ın yüklediği imajı da kullanabilirsin: GitHub'da bir CI çalışmasının sayfasındaki
`ankaros-qemu-aarch64` dosyasında aynı iki dosya bulunur.

## 2. Sanal makineyi oluştur

UTM'de **Create a New Virtual Machine** seç, sihirbazda:

| Adım | Seçim |
|---|---|
| Start | **Virtualize** |
| Operating System | **Linux** |
| Linux | **Use Apple Virtualization** kutusu boş kalır (GPU hızlandırması yalnızca QEMU arka ucunda var). **Boot from kernel image** işaretlenir. |
| Kernel | `~/ankaros-utm/Image` |
| Initial Ramdisk | boş |
| Root FS Image | `~/ankaros-utm/rootfs.img` |
| Boot Arguments | `rootwait root=/dev/vda console=ttyAMA0` |
| Hardware | Bellek 2048 MB, 4 çekirdek. **Enable hardware OpenGL acceleration** işaretlenir. |
| Storage | Ek disk gerekmez |
| Summary | Ad: `ankaros`. **Open VM Settings** işaretlenir, ayarlar bir sonraki adımda değişecek. |

## 3. Ayarlar

Sihirbazdan sonra açılan ayarlar penceresinde:

- **Display:** Emulated Display Card **virtio-gpu-gl-pci (GPU Supported)** olmalı.
- **Input:** ankaros çekirdeğinde şimdilik USB klavye ve fare sürücüsü yok, virtio girdi aygıtları var.
  **QEMU → Arguments** bölümüne şu iki satırı ekle:

  ```text
  -device virtio-keyboard-pci
  -device virtio-tablet-pci
  ```

- **Network:** Shared Network (varsayılan) yeterli.

Kaydet ve sanal makineyi başlat. Birkaç saniye içinde ankaros masaüstü gelmeli.

## 4. GPU hızlandırması çalışıyor mu?

Compositor'ın günlüğünde hangi sürücünün kullanıldığı görünür. Terminal'i aç (alt çubukta) ve:

```bash
grep -i -m3 "renderer\|virgl\|softpipe\|llvmpipe" /var/log/ankaros-compositor.log
cat /sys/kernel/debug/dri/0/name 2>/dev/null
```

`virgl` görünüyorsa hızlandırma açıktır, `softpipe` görünüyorsa sanal makine 2B GPU ile açılmıştır:
Display ayarını ve sihirbazdaki OpenGL kutusunu kontrol et.

## Sorunlar

| Belirti | Neden ve çözüm |
|---|---|
| Ekran siyah, konsol yazısı da yok | Display kartı `virtio-gpu-gl-pci` değil ya da çekirdek açılmadı. **Serial** konsolu eklenip açılış iletileri okunabilir. |
| `VFS: Unable to mount root fs` | Root FS diski VirtIO arayüzünde değil. **Drives** altında arayüzü **VirtIO** yap (`/dev/vda`). |
| Masaüstü geliyor, fare ve klavye çalışmıyor | 3. adımdaki QEMU argümanları eksik. |

## Yeni imajı denemek

UTM, kök dosya sistemini sanal makinenin kendi klasörüne kopyalar. Yeni bir imajı denemek için en
kolayı sanal makineyi silip bu rehberi baştan uygulamaktır. Kernel dosyası ayarlardan değiştirilebilir.
