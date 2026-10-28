# Değişiklik günlüğü

Bu dosya [Keep a Changelog](https://keepachangelog.com/tr-TR/1.1.0/) biçimini, sürümler
[Semantic Versioning](https://semver.org/lang/tr/) kurallarını izler.

## [Yayınlanmamış]

### Eklendi

- Grafik yığını: Qt 6.9 (eglfs, QML, Wayland compositor), Mesa (virgl, softpipe), libinput, weston-terminal
- `ankaros-shell` paketi: kabuk ankaros'ta tam ekran çalışıyor
- Türkçe dil ayarı (tr_TR.UTF-8) ve İstanbul saat dilimi
- `ankaros-compositor`: Qt Wayland Compositor ile Wayland sunucusu ve tam ekran masaüstü. Pencereler
  ankaros çerçevesiyle açılıyor, taşınıyor, öne geliyor, küçültülüp büyütülüyor, kapanıyor. Alt
  çubuktaki göstergeler gerçek pencerelerden geliyor, Terminal alt çubuktan açılıyor.
- Kabuğa pencere çerçevesi (`WindowChrome`, `WindowTitleBar`) ve pencere katmanı
- Masaüstü açılışta kendiliğinden başlıyor (`/etc/init.d/S90ankaros-desktop`)
- Ayarlar › Görünüm: tema, pencere düğmelerinin konumu (sol/sağ) ve stili (simge/renkli daire).
  Bütün pencerelere anında uygulanıyor, `~/.config/ankaros/shell.conf`'ta saklanıyor.
- `run-qemu.sh --gui` ve ekran görüntüsü alan otomatik test betiği `scripts/qemu-test.py`
- CI'da açılış testi: her imaj derlemesinden sonra imaj QEMU'da açılır, masaüstünün ekrana geldiği
  denetlenir, ekran görüntüsü yüklenir
- UTM'de GPU hızlandırmalı sanal makineyi tek komutla kuran `scripts/utm-olustur.sh` ve rehberi (`docs/utm.md`)

### Değişti

- CI'daki imaj derlemesi ayrı iş akışına (`imaj.yml`) taşındı. Yalnızca imajı etkileyen dosyalar
  değişince çalışıyor, PR'lar onu beklemiyor.

## [0.1.0] - 2026-10-28

Faz 0: QEMU'da açılan en küçük ankaros sistemi ve Mac'te çalışan masaüstü kabuğu.

### Eklendi

- Buildroot 2026.02 LTS tabanlı derleme sistemi ve `ankaros_qemu_aarch64_defconfig`
- Linux 6.18, glibc, BusyBox ve eudev ile en küçük sistem
- `scripts/build.sh`, `scripts/progress.sh`, `scripts/run-qemu.sh`
- Masaüstü kabuğu (`Ankaros.Shell`), Figma'daki "B · Ankara Taşı" tasarımıyla: buzlu cam alt çubuk
  (başlat, arama, uygulamalar, durum, saat), aramalı başlat paneli (sabitlenmiş uygulamalar, önerilenler),
  açık/koyu tema, gömülü Inter yazı tipi, Qt Shapes ile çizilen simgeler
- Kabuğu Mac'te gösteren önizleme uygulaması ve `scripts/shell-preview.sh`
- Kod stili ayarları (clang-format, clang-tidy, qmlformat, EditorConfig)
- MIT lisansı
- Ekip belgeleri: katkı rehberi, mimari, geliştirme ortamı, yol haritası, karar kayıtları

### Düzeltildi

- `run-qemu.sh`: OrbStack klasöründeki imaj açılırken çıkan disk kilidi hatası

[Yayınlanmamış]: https://github.com/DyrALT/ankaros/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/DyrALT/ankaros/releases/tag/v0.1.0
