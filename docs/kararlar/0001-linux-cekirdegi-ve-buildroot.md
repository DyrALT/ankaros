# 0001: Linux çekirdeği ve Buildroot

- **Durum:** Kabul edildi
- **Tarih:** 2026-10-28

## Bağlam

ankaros bir hobi projesi. Hedefimiz hızlı bir sistem ve kendimize özel bir masaüstü deneyimi.
Çekirdek yazmak bu hedeflere doğrudan katkı sağlamıyor ve yıllar alabilir.

## Değerlendirilen seçenekler

- **Sıfırdan çekirdek:** Çok öğretici, ama sürücüler, dosya sistemleri ve ağ yığını yüzünden masaüstüne
  ulaşmak çok uzun sürer.
- **seL4 / Zircon gibi mikroçekirdekler:** Mimari olarak ilginç, ama sürücü ve servisleri kendimiz yazmamız gerekir.
- **FreeBSD:** Düzenli bir taban, ama grafik yığını ve Wayland desteği Linux kadar olgun değil.
- **Linux:** En geniş donanım desteği ve en olgun grafik yığını (DRM/KMS, Mesa, Wayland).

Linux'u bir sisteme dönüştürmek için: LFS (çok elle iş), Yocto (güçlü ama karmaşık), mevcut bir dağıtımı
temel almak (hızlı ama "kendi sistemimiz" olmaz) ve **Buildroot** (sade, tek yapılandırma dosyası).

## Karar

Linux çekirdeği ve Buildroot. Buildroot bir LTS sürümüne sabitlenmiş git submodule olarak tutulur,
ankaros'a özel her şey BR2_EXTERNAL ağacında (`external/`) durur.

## Sonuçlar

- Çekirdek ve temel araçlar hazır geliyor. Emeğimizi kabuk, compositor ve sistem servislerine veriyoruz.
- Buildroot'un paket yöneticisi yok, sistem tek bir imaj olarak derlenir. Kendi paket yöneticimiz
  Faz 3'te ayrıca tasarlanacak.
- Buildroot'ta yapılandırma değişiklikleri (özellikle araç zinciri) temiz derleme gerektirebilir.
