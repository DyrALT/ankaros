# Yol haritası

ankaros'un hedefi: **hızlı** ve **kendine özel bir masaüstü deneyimi** sunan bir sistem.

## Faz 0: Açılan en küçük sistem (`v0.1.0`), tamamlandı

- [x] Derleme ortamı: OrbStack'te Debian arm64 makinesi
- [x] Buildroot 2026.02 LTS, BR2_EXTERNAL yapısı
- [x] Linux 6.18 + glibc + BusyBox + eudev yapılandırması
- [x] Derleme, ilerleme ve QEMU betikleri
- [x] Kod stili ayarları ve ekip belgeleri
- [x] İmajın QEMU'da açılıp giriş istemi göstermesi (açılıştan girişe ~10 sn, 34 MB RAM, 18 MB disk)
- [x] CI'da imaj derlemesi (GitHub'ın arm64 makinesinde, ilk derleme ~40 dk)
- [x] Kabuk prototipi Mac'te çalışıyor (`shell/preview`)

## Faz 1: Grafik masaüstü (`v0.2.0`), tamamlandı

- [x] Figma'da masaüstü tasarımı: dört varyasyon çizildi, B · Ankara Taşı seçildi
- [x] Kabuğun QML ile tasarıma göre yazılması (Mac önizlemesinde)
Ayrıntılı plan: [faz-1-plani.md](faz-1-plani.md)

- [x] Figma'da pencere başlık çubuğu (düğme konumu ve stili varyantları) ve Görünüm ayarları ekranı
- [x] A: Buildroot'a Qt 6 (qtbase, qtdeclarative, qtwayland), Mesa (virgl, softpipe), libinput, weston-terminal eklenmesi
- [x] B: Qt Wayland Compositor ile compositor: pencere açma, taşıma, odak, kapatma, ankaros başlık çubuğu
- [x] C: Pencere katmanı, gerçek çalışma göstergeleri, uygulama açma, küçültme/büyütme, Görünüm ayarları
  - Uygulama listesinin dosyadan okunması Faz 2'ye kaldı
- [x] D: Açılışta giriş istemi yerine doğrudan masaüstü
- [x] E: QEMU'da otomatik açılış testi, UTM'de GPU hızlandırmalı test
  - [x] CI'da açılış testi (`qemu-test.py --expect-desktop`)
  - [x] UTM rehberi ([utm.md](utm.md))
  - [x] UTM'de virgl ile GPU hızlandırmasının doğrulanması (`scripts/utm-olustur.sh`)
  - [x] `v0.2.0`

## Faz 2: Sistem servisleri (`v0.3.0`), tamamlandı

Ayrıntılı plan: [faz-2-plani.md](faz-2-plani.md)

- [x] A: Uygulama listesinin `/etc/ankaros/apps.json`'dan okunması
- [x] B: Kendi init sistemimiz (`ankaros-init`, C++)
- [x] C: Figma'da giriş ekranı, kilit ekranı, güç menüsü, Kullanıcı ve Ağ ayarları
- [x] D: Oturum yönetimi: kullanıcı hesabı, giriş ve kilit ekranı, otomatik giriş
- [x] E: Kablolu ağ servisi ve Ayarlar › Ağ
- [x] F: Açılış süresinin ölçülüp raporlanması ([acilis-suresi.md](acilis-suresi.md))
- [x] `v0.3.0`

## Faz 3: Paketler ve uygulamalar (`v0.4.0`), sıradaki

Ayrıntılı plan: [faz-3-plani.md](faz-3-plani.md)

- [x] A: Ortak modüller: Ankaros.UI (bileşenler) ve Ankaros.System (servis istemcileri)
- [x] B: Figma'da Terminal, Ayarlar uygulaması ve Paket Merkezi
- [x] C: Ayarlar uygulaması (Duvar kağıdı, Alt çubuk, Hakkında sayfaları)
- [x] D: Terminal (libvterm)
- [ ] E: Uygulama paketi (`.ankpkg`), paket servisi, `ank` komutu, imzalı depo
- [ ] F: Paket Merkezi
- [ ] `v0.4.0`

## Faz 4: Dağıtım (`v0.5.0`)

- [ ] Kurulum programı
- [ ] ISO imajı, gerçek donanımda test
