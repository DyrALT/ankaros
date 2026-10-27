# Yol haritası

ankaros'un hedefi: **hızlı** ve **kendine özel bir masaüstü deneyimi** sunan bir sistem.

## Faz 0: Açılan en küçük sistem (`v0.1.0`), devam ediyor

- [x] Derleme ortamı: OrbStack'te Debian arm64 makinesi
- [x] Buildroot 2026.02 LTS, BR2_EXTERNAL yapısı
- [x] Linux 6.18 + glibc + BusyBox + eudev yapılandırması
- [x] Derleme, ilerleme ve QEMU betikleri
- [x] Kod stili ayarları ve ekip belgeleri
- [ ] İmajın QEMU'da açılıp giriş istemi göstermesi
- [ ] CI'da imaj derlemesi
- [ ] Kabuk prototipi Mac'te çalışıyor (`shell/preview`)

## Faz 1: Grafik masaüstü (`v0.2.0`)

- [ ] Figma'da masaüstü tasarımı: panel, başlatıcı, renkler, tipografi
- [ ] Kabuğun QML ile tasarıma göre yazılması (Mac önizlemesinde)
- [ ] Buildroot'a Qt 6 (qtbase, qtdeclarative, qtwayland), Mesa (virgl), libinput eklenmesi
- [ ] Qt Wayland Compositor ile compositor: pencere açma, taşıma, kapatma
- [ ] Açılışta giriş istemi yerine doğrudan masaüstü
- [ ] UTM'de GPU hızlandırmalı test

## Faz 2: Sistem servisleri (`v0.3.0`)

- [ ] Kendi init sistemimiz (C++)
- [ ] Oturum yönetimi, ağ ayarları arayüzü
- [ ] Açılış süresi ölçümü ve optimizasyonu

## Faz 3: Paketler ve uygulamalar (`v0.4.0`)

- [ ] Paket formatı ve paket yöneticisi
- [ ] Temel uygulamalar: terminal, dosya yöneticisi, ayarlar

## Faz 4: Dağıtım (`v0.5.0`)

- [ ] Kurulum programı
- [ ] ISO imajı, gerçek donanımda test
