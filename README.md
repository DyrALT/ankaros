# ankaros

Linux çekirdeği üzerine kurulu, **hızlı** ve **kendine özel bir masaüstü deneyimi** hedefleyen hobi
işletim sistemi.

| | |
|---|---|
| Taban | Linux 6.18, Buildroot 2026.02 LTS, glibc, BusyBox, eudev |
| Mimari | aarch64 (Apple Silicon üzerinde donanım hızlandırmalı sanal makine) |
| Masaüstü | C++20 + QML, Qt 6, Qt Wayland Compositor (Faz 1) |
| Durum | Faz 0: açılan en küçük sistem ([yol haritası](docs/yol-haritasi.md)) |

## Hızlı başlangıç

Gereksinimler: Apple Silicon Mac, [OrbStack](https://orbstack.dev), Homebrew. Kurulumun ayrıntıları
[docs/gelistirme-ortami.md](docs/gelistirme-ortami.md) belgesinde.

```bash
git clone --recursive <repo-adresi> && cd ankaros
./scripts/build.sh        # ilk derleme uzun sürer (araç zinciri dahil)
./scripts/progress.sh     # derleme ilerlemesi (canlı: watch -n 5 ./scripts/progress.sh)
./scripts/run-qemu.sh     # kullanıcı: root, şifre yok. Çıkış: Ctrl-a x
./scripts/shell-preview.sh  # masaüstü kabuğunu Mac'te bir pencerede açar
```

![Kabuk önizlemesi](docs/gorseller/kabuk-acik.png)

## Repo yapısı

| Yol | İçerik |
|---|---|
| `buildroot/` | Buildroot (git submodule, sürüme sabitli) |
| `external/` | ankaros'a özel her şey (BR2_EXTERNAL ağacı): defconfig, kart dosyaları, paketler |
| `shell/` | Masaüstü kabuğu: `Ankaros.Shell` QML modülü ve Mac önizleme uygulaması |
| `scripts/` | Derleme, ilerleme ve çalıştırma betikleri |
| `docs/` | Belgeler: [mimari](docs/mimari.md), [karar kayıtları](docs/kararlar/), [tümü](docs/README.md) |

## Katkı

Dal stratejisi, commit kuralları ve kod stili için [CONTRIBUTING.md](CONTRIBUTING.md).
Değişiklikler [CHANGELOG.md](CHANGELOG.md) dosyasında.

## Lisans

ankaros'un kendi kodu ve belgeleri [MIT lisansıyla](LICENSE) yayınlanır. Buildroot (`buildroot/`) ve
derlenen imajdaki paketler (Linux, Qt, glibc, BusyBox vb.) kendi lisanslarına tabidir.
Ayrıntılar: [docs/kararlar/0005-mit-lisansi.md](docs/kararlar/0005-mit-lisansi.md)
