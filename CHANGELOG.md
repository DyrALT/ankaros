# Değişiklik günlüğü

Bu dosya [Keep a Changelog](https://keepachangelog.com/tr-TR/1.1.0/) biçimini, sürümler
[Semantic Versioning](https://semver.org/lang/tr/) kurallarını izler.

## [Yayınlanmamış]

### Eklendi

- Buildroot 2026.02 LTS tabanlı derleme sistemi ve `ankaros_qemu_aarch64_defconfig`
- Linux 6.18, glibc, BusyBox ve eudev ile en küçük sistem
- `scripts/build.sh`, `scripts/progress.sh`, `scripts/run-qemu.sh`
- Masaüstü kabuğu prototipi (`Ankaros.Shell`): duvar kağıdı, panel, saat, aramalı uygulama başlatıcı,
  açık/koyu tema
- Kabuğu Mac'te gösteren önizleme uygulaması ve `scripts/shell-preview.sh`
- Kod stili ayarları (clang-format, clang-tidy, qmlformat, EditorConfig)
- MIT lisansı
- Ekip belgeleri: katkı rehberi, mimari, geliştirme ortamı, yol haritası, karar kayıtları

### Düzeltildi

- `run-qemu.sh`: OrbStack klasöründeki imaj açılırken çıkan disk kilidi hatası
