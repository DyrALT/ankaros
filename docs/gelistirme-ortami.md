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
kabuğu tam ekran başlatır. `weston-terminal` Wayland terminalidir, compositor içinde çalışır. Bu modda
sanal GPU 2D'dir ve Mesa yazılımsal çizer (softpipe): çalışır ama yavaştır. GPU hızlandırmalı test
UTM ile yapılır: `./scripts/utm-olustur.sh`, ayrıntılar [utm.md](utm.md).

### Masaüstü

Masaüstü açılışta `ankaros` kullanıcısıyla (şifre `ankaros`) otomatik girişle açılır. Oturumu
oturum yöneticisi `ankaros-session` başlatır, bkz. [oturum.md](oturum.md). Alt çubuktaki Terminal simgesi
`weston-terminal`'i açar. Seri konsolda (root):

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
alır. Masaüstü `--wait` saniye içinde gelmezse 1 ile çıkar. CI her imaj derlemesinden sonra bunu
çalıştırır, ekran görüntüsünü ve günlüğü `acilis-testi` adıyla yükler. Ölçüt piksel karşılaştırması
değildir: siyah konsol ekranı ile renkli duvar kâğıdı birbirinden ayrılır, saat ya da tasarım değişince
test bozulmaz.

Komut çalıştırıp fare tıklaması ve tuş da gönderilebilir. `--actions` içindeki eylemler `;` ile ayrılır:
`click X Y` (1440x900 koordinatları), `sleep SN`, `key TUŞLAR` (QEMU adlarıyla: `ret`, `esc`, `meta_l-l`),
`type METİN` (küçük harf ve rakam), `run KOMUT` (seri konsolda root olarak), `shot DOSYA` (ara ekran
görüntüsü).

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
- **QEMU'da `Failed to lock byte 100`**: İmaj OrbStack'in paylaşılan klasöründen okunuyor ve bu klasör
  dosya kilitlerini desteklemiyor. `run-qemu.sh` kilitlemeyi bu yüzden kapatır (`file.locking=off`).
  QEMU'yu elle çalıştırıyorsanız aynı seçeneği ekleyin.
- **QEMU'da `hvf` hatası**: Intel Mac'te çalışmaz. `-accel hvf` yerine `-accel tcg` kullanın (çok yavaş).
