# Geliştirme ortamı

ankaros macOS (Apple Silicon) üzerinde geliştirilir.

1. **İşletim sistemi imajı:** Buildroot, Linux üzerinde derleme yapar. Bu yüzden derleme OrbStack'teki
   bir Linux makinesinde yapılır, imaj Mac'te QEMU ile açılır.

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

`run-qemu.sh` QEMU'yu Apple'ın donanım hızlandırmasıyla (`-accel hvf`) çalıştırır. Bu yüzden misafir
sistem neredeyse yerel hızda açılır. Ek QEMU argümanları betiğe eklenebilir.

## Sık karşılaşılan sorunlar

- **`orb: machine not found`**: Derleme makinesi oluşturulmamış ya da adı farklı. `orb list` ile kontrol edin.
- **Derleme "No space left on device" ile duruyor**: Mac'te disk dolmuştur. OrbStack makinesinin diski
  Mac'in diskinden yer kullanır.
- **QEMU'da `hvf` hatası**: Intel Mac'te çalışmaz. `-accel hvf` yerine `-accel tcg` kullanın (çok yavaş).
