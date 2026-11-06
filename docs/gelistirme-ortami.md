# Geliştirme ortamı

ankaros macOS (Apple Silicon) üzerinde geliştirilir. İki ayrı iş akışı var:

1. **İşletim sistemi imajı:** Buildroot, Linux üzerinde derleme yapar. Bu yüzden derleme OrbStack'teki
   bir Linux makinesinde yapılır, imaj Mac'te QEMU ile açılır.
2. **Masaüstü kabuğu:** QML arayüzü doğrudan Mac'te, yerel bir pencerede çalıştırılarak geliştirilir.
   İmaj derlemeye gerek yoktur.

## Gereksinimler

- Apple Silicon Mac (M1 veya üstü), en az 16 GB RAM
- **En az 60 GB boş disk** (Faz 0 için ~15 GB yeter, Qt 6 eklendiğinde 40 GB'ı geçer)
- [Homebrew](https://brew.sh)
- [OrbStack](https://orbstack.dev)

## 1. Repoyu klonlamak

Buildroot bir submodule olduğu için `--recursive` gerekir:

```bash
git clone --recursive <repo-adresi> ~/Desktop/ankaros
```

Submodule'ü sonradan almak için: `git submodule update --init`

## 2. Mac araçları

```bash
brew install qemu qt cmake ninja clang-format shellcheck
```

## 3. Derleme makinesi

```bash
orb create debian:bookworm ankaros-build
orb -m ankaros-build -u root bash -c 'apt-get update && apt-get install -y build-essential file wget cpio unzip rsync bc git python3 libncurses-dev perl patch bzip2 xz-utils ca-certificates'
```

OrbStack, Mac'teki `/Users` klasörünü makinede aynı yolla gösterir. Böylece repo iki tarafta da
`~/Desktop/ankaros` yolundadır.

Buildroot çıktısı ise bilerek **makinenin kendi diskinde** (`~/ankaros-output`) tutulur, çünkü:

- Buildroot büyük/küçük harfe duyarlı bir dosya sistemi ister, macOS'un dosya sistemi (APFS) varsayılan
  olarak duyarlı değildir.
- Paylaşılan klasör üzerinden derlemek çok daha yavaştır.

Mac'ten bu dosyalara `~/OrbStack/ankaros-build/home/<kullanıcı>/ankaros-output` yolundan ulaşılır.
İndirilen kaynak paketleri de `~/ankaros-dl` altında önbelleğe alınır, ccache önbelleği
`~/.buildroot-ccache` altındadır.

## 4. İmajı derleyip açmak

```bash
./scripts/build.sh            # ilk derleme 30-60 dk (araç zinciri dahil), sonrakiler dakikalar
./scripts/progress.sh         # ilerleme (canlı: makinede watch -n 5 ./scripts/progress.sh)
./scripts/run-qemu.sh         # kullanıcı: root, şifre yok. Çıkış: Ctrl-a ardından x
```

`build.sh`'e verilen argümanlar doğrudan Buildroot'un `make` komutuna gider:

| Komut | Ne yapar |
|---|---|
| `./scripts/build.sh menuconfig` | Yapılandırmayı menüden düzenler |
| `./scripts/build.sh savedefconfig` | Değişiklikleri `external/configs/` altındaki defconfig'e yazar |
| `./scripts/build.sh linux-menuconfig` | Çekirdek yapılandırması |
| `./scripts/build.sh <paket>-rebuild` | Tek bir paketi yeniden derler |
| `./scripts/build.sh clean` | Çıktıyı siler (araç zinciri de yeniden derlenir!) |

### Grafik mod

```bash
./scripts/run-qemu.sh --gui -snapshot
```

Seri konsolun yanında bir ekran penceresi açılır. Giriş yaptıktan sonra `ankaros-shell-preview` komutu
kabuğu tam ekran başlatır. Bu modda
sanal GPU 2D'dir ve Mesa yazılımsal çizer (softpipe): çalışır ama yavaştır. GPU hızlandırmalı test
UTM ile yapılır: `./scripts/utm-olustur.sh`, ayrıntılar [utm.md](utm.md).

### Masaüstü

Masaüstü açılışta `ankaros` kullanıcısıyla (şifre `ankaros`) otomatik girişle açılır. Oturumu
oturum yöneticisi `ankaros-session` başlatır, bkz. [oturum.md](oturum.md). Alt çubuktaki Terminal simgesi
ankaros'un kendi terminalini (`ankaros-terminal`, [terminal.md](terminal.md)) açar. Seri konsolda (root):

```bash
cat /var/log/ankaros-compositor.log        # compositor'ın çıktısı
ankarosctl restart ankaros-session         # masaüstünü yeniden başlat (ör. ayar dosyasını elle değiştirdikten sonra)
ankarosctl session logout                  # giriş ekranına dön
ankarosctl status                          # bütün servislerin durumu
```

Qt uygulamaları Wayland istemcisi olarak açılabilir: `QT_QPA_PLATFORM=wayland ankaros-shell-preview`.

### Otomatik test

```bash
./scripts/qemu-test.py --expect-desktop --wait 120 --screenshot masaustu.png
```

Açılış testi: imajı ekransız açar, masaüstü ekrana gelene kadar birkaç saniyede bir ekran görüntüsü
alır, masaüstü gelince açılış süresi raporunu ([acilis-suresi.md](acilis-suresi.md)) yazdırır.
Masaüstü `--wait` saniye içinde gelmezse 1 ile çıkar. CI her imaj derlemesinden sonra bunu
çalıştırır, ekran görüntüsünü ve günlüğü `acilis-testi` adıyla yükler. Ölçüt piksel karşılaştırması
değildir: siyah konsol ekranı ile renkli duvar kâğıdı birbirinden ayrılır, saat ya da tasarım değişince
test bozulmaz.

Komut çalıştırıp fare tıklaması ve tuş da gönderilebilir. `--actions` içindeki eylemler `;` ile ayrılır:
`click X Y` (1440x900 koordinatları), `sleep SN`, `key TUŞLAR` (QEMU adlarıyla: `ret`, `esc`, `meta_l-l`),
`type METİN` (küçük harf, rakam ve `. / - ,`, Türkçe Q düzenine göre), `run KOMUT` (seri konsolda root
olarak), `link on|off` (sanal ağ kablosu), `shot DOSYA` (ara ekran görüntüsü).

```bash
./scripts/qemu-test.py --command "ankaros-shell-preview" --wait 25 --screenshot ekran.png
./scripts/qemu-test.py --wait 20 \
  --actions "click 326 860; sleep 15" --screenshot terminal.png   # alt çubuktaki Terminal'e tıkla
./scripts/qemu-test.py --expect-desktop --actions \
  "key meta_l-l; sleep 10; type yanlis; key ret; sleep 10" --screenshot kilit.png   # kilit ekranı
```

Seri konsol çıktısı ve compositor günlüğü `ekran.log` dosyasına yazılır. QEMU'da GPU olmadığı için
bulanıklaştırma gibi efektler yavaştır: kilit ekranı gibi bir değişiklikten sonra ekran görüntüsü
almadan önce birkaç saniye bekleyin. Mac'te donanım
hızlandırması (`hvf`) kullanılır. KVM olmayan Linux'ta (CI, OrbStack'teki derleme makinesi) işlemci
yazılımla taklit edilir (`tcg`), o zaman da masaüstü yaklaşık 20 saniyede gelir. `--accel tcg` ile
Mac'te de denenebilir.

`run-qemu.sh` QEMU'yu Apple'ın donanım hızlandırmasıyla (`-accel hvf`) çalıştırır. Bu yüzden misafir
sistem neredeyse yerel hızda açılır. Ek QEMU argümanları betiğe eklenebilir. Örneğin
`./scripts/run-qemu.sh -snapshot` imajda yapılan değişiklikleri kapanışta atar, deneme için kullanışlıdır.

### x86_64 bilgisayarda

Mac dışındaki adımlar aynıdır, derleme Linux'ta doğrudan çalışır ([karar 0018](kararlar/0018-x86-64-imaji.md)).
Windows için aşağıdaki [Windows'ta](#windowsta) bölümüne bakın. Derleme için bölüm 3'teki paketler kurulur, ardından:

```bash
./scripts/build.sh         # bilgisayarın mimarisini derler, çıktı ~/ankaros-output
./scripts/run-qemu.sh --gui   # KVM varsa donanım hızlandırmasıyla açar
./scripts/qemu-test.py --expect-desktop --wait 120 --screenshot masaustu.png
```

- `/dev/kvm`'ye yazma izni yoksa QEMU işlemciyi taklit eder, çok yavaştır. Kullanıcıyı `kvm` grubuna ekleyin.
- Başka bir mimariyi derlemek için `ANKAROS_ARCH=x86_64 ./scripts/build.sh`. Buildroot o mimarinin araç zincirini
  kendisi derler, Mac'teki arm64 makinede de çalışır. Çıktı `~/ankaros-output-x86_64`'e gider. İmajı `IMAGES`
  değişkeniyle verin. Mac'te işlemci taklit edildiği için çok yavaş açılır.
- İmaj yalnızca sanal makine içindir, gerçek bilgisayarda açılmaz (Faz 7). UTM Mac'e özgüdür.

### Windows'ta

Windows 11'de x86_64 imajı derlenir ve QEMU'da WHPX (Windows Hypervisor Platform) ile donanım
hızlandırmalı açılır. aarch64 imajı da açılır ama işlemci taklit edildiği için çok yavaştır.

1. **Gerekenler:** WSL2'de Ubuntu (derleme için), Windows'ta [QEMU](https://www.qemu.org/download/#windows)
   ve "Windows Hypervisor Platform" özelliği (Windows Özellikleri'nden açılır).
2. **Derleme (WSL'de):** Linux'taki adımlar aynıdır. WSL'in Windows'tan miras aldığı `PATH` boşluk içerdiği ve
   Buildroot bunu reddettiği için temiz bir `PATH` ile çalıştırın. Ubuntu 25.10 ve sonrasında `install`
   komutu uutils sürümüdür ve Buildroot onu kabul etmez, `/usr/bin/gnuinstall`'a bağlantı veren bir klasörü
   `PATH`'in başına koyun:

   ```bash
   mkdir -p ~/shim && ln -sf /usr/bin/gnuinstall ~/shim/install
   ANKAROS_ARCH=x86_64 PATH=$HOME/shim:/usr/local/bin:/usr/bin:/bin ./scripts/build.sh
   ```

   Derleme saatler sürer ve çok bellek ister. WSL'e varsayılandan fazla bellek ve işlemci vermek için
   `%UserProfile%\.wslconfig` dosyasında `[wsl2]` altına `memory=` ve `processors=` yazın. Derlemeyi
   `setsid nohup ... &` ile başlatırsanız terminal kapanınca durmaz.
3. **Açma (PowerShell'de):**

   ```powershell
   .\scripts\run-qemu.ps1 -Arch x86_64
   ```

   Betik imajı WSL'den `%UserProfile%\ankaros-images-x86_64`'e kopyalar (QEMU WSL'in ağ dosya sisteminde
   çalışmaz), veri diskini `%LocalAppData%\ankaros\veri.qcow2` olarak oluşturur ve QEMU'yu açar.
   `-Snapshot` imajda ve veri diskinde (kurulan katmanlar ve uygulamalar dahil) yapılan değişiklikleri
   kapanışta atar, `-Refresh` yeni derlemeyi yeniden kopyalar. `-Width` ve `-Height` ekran çözünürlüğünü
   (varsayılan 1440x900) verir: betik bunu çekirdek argümanı `ankaros.mode=GENxYÜK` olarak taşır,
   `/etc/profile.d/ankaros-grafik.sh` okuyup KMS yapılandırmasını `/run/user/<uid>/kms.json` olarak üretir.
   Ek QEMU argümanları `-Extra @('-qmp', 'tcp:127.0.0.1:4444,server=on,wait=off')` biçiminde verilir
   (tek başına `-qmp ...` yazılırsa PowerShell onu betiğin kendi parametrelerine bağlamaya çalışır).

Betiğin seçtikleri ve nedenleri:

- **`-vga none`:** q35'in varsayılan standart VGA'sı kalırsa Mesa onu seçer ve sanal GPU'yu kullanmadan
  yazılımla çizer (softpipe). Bu standart kart 3B çizim bilmez, bu yüzden her kare işlemcide çizilir:
  tıklamalara geç cevap gelir, bulanıklaştırma gibi efektler yavaşlar ve yavaş çizimde olay döngüsü
  tıkandığı için kısa tıklamalar kaybolabilir. Bunu kapatmak masaüstünü belirgin biçimde hızlandırır.
- **virgl (`virtio-gpu-gl`, `gtk,gl=on`):** Windows'ta QEMU'nun bu desteği vardır. Compositor
  ekranı ana bilgisayarın GPU'suyla çizer. `-NoGl` yazılımla çizime döner.
- **`zoom-to-fit` kapalı:** Pencere ekranla aynı boyutta gösterilir. Ölçeklenirse görüntü bulanıklaşır, `-Fit`
  ile açılır.
- Pencerenin içindeki uygulamalar (Ayarlar, Terminal) da sanal GPU'yla çizer. Bunun için Mesa
  `EGL_WL_bind_wayland_display` desteğiyle derlenir ve compositor `wayland-egl` ile `linux-dmabuf`
  eklentilerini yükler ([faz-4-plani.md](faz-4-plani.md)). Eski bir imajda ya da bu ayar olmadan
  Ayarlar › Hakkında'da "Ekran kartı: softpipe" görünür.
- **Açılış testi:** `scripts/qemu-test.py`'nin `--accel` seçeneği yalnızca `hvf`, `kvm` ve `tcg` kabul eder, `whpx`
  yoktur. Windows'ta bu betik işlemciyi yazılımla taklit eder (yavaş). Masaüstünü hızlı denemek için
  `run-qemu.ps1` kullanılır.

## 5. Masaüstü kabuğunu Mac'te çalıştırmak

Kabuk (QML arayüzü) imaj derlemeden, doğrudan Mac'te bir pencerede geliştirilir:

```bash
./scripts/shell-preview.sh
```

Betik ilk seferde CMake ile yapılandırır, kabuğu derler ve önizleme penceresini açar. Pencerede
`Cmd+R` kabuğu yeniden derleyip yeniden açar, `Cmd+D` açık/koyu mod arasında geçiş yapar,
`Ctrl+Cmd+F` tam ekrana geçer.
Derleme çıktısı repo içinde `build/shell` altındadır (git'e girmez).

Ayrıntılar ve tasarım akışı için [tasarim-akisi.md](tasarim-akisi.md).

## Sık karşılaşılan sorunlar

- **`orb: machine not found`**: Derleme makinesi oluşturulmamış ya da adı farklı. `orb list` ile kontrol edin.
- **Derleme "No space left on device" ile duruyor**: Mac'te disk dolmuştur. OrbStack makinesinin diski
  Mac'in diskinden yer kullanır.
- **Paket derlenirken `stdlib.h: No such file or directory`**: Derleme makinesine sistem `cmake`'i kurulmuşsa
  Buildroot onu seçer ve 3.25 sürümü başlık yollarını yanlış hesaplar. Makineden kaldırın ya da Buildroot'un
  kendisininkini derleme makinesindeki tam yoluyla verin (`~` değil, Mac'ten çalıştırınca Mac'e açılır, önce
  `./scripts/build.sh host-cmake` gerekir, klasör mimariye göre değişir):
  `./scripts/build.sh BR2_CMAKE=/home/<kullanıcı>/ankaros-output/host/bin/cmake`. Sonra paketi
  `<paket>-dirclean` ile temizleyip yeniden derleyin.
- **QEMU'da `Failed to lock byte 100`**: İmaj OrbStack'in paylaşılan klasöründen okunuyor ve bu klasör
  dosya kilitlerini desteklemiyor. `run-qemu.sh` kilitlemeyi bu yüzden kapatır (`file.locking=off`).
  QEMU'yu elle çalıştırıyorsanız aynı seçeneği ekleyin. Windows'ta aynı hata imaj WSL'in ağ yolundan
  (`\\wsl.localhost\...`) açılınca çıkar, `run-qemu.ps1` bu yüzden imajı Windows tarafına kopyalar.
- **Dosya silindi ya da taşındı ama imajda eski hali duruyor**: Buildroot, repodaki `system/`, `shell/` ve
  `compositor/` klasörlerini derleme klasörüne `rsync` ile kopyalar ve silinen dosyaları orada bırakır.
  Taşınan bir başlık dosyasının eski kopyası derlemeye karışabilir, imaja eski bir servis dosyası
  girebilir. Bir dosyayı sildikten ya da taşıdıktan sonra paketin derleme klasörünü temizleyin:
  `./scripts/build.sh ankaros-system-dirclean all` (paket adı değişir). CI her seferinde temiz derler.
- **QEMU'da `hvf` hatası**: Intel Mac'te çalışmaz. `-accel hvf` yerine `-accel tcg` kullanın (çok yavaş).
- **Windows'ta QEMU'da `whpx` hatası**: "Windows Hypervisor Platform" özelliği kapalıdır. Windows Özellikleri'nden
  açıp bilgisayarı yeniden başlatın. Açmak istemezseniz `run-qemu.ps1 -Accel tcg` yazılımla açar (çok yavaş).
