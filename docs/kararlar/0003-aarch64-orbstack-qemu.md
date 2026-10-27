# 0003: aarch64 hedefi, OrbStack ile derleme, QEMU ile test

- **Durum:** Kabul edildi
- **Tarih:** 2026-10-28

## Bağlam

Geliştirme Apple Silicon Mac'lerde, sistem sanal makinede çalıştırılarak yapılıyor. Buildroot sadece
Linux üzerinde derleme yapabiliyor ve büyük/küçük harfe duyarlı bir dosya sistemi istiyor.

## Karar

- **Hedef mimari aarch64.** Apple Silicon'da sanal makine donanım hızlandırmasıyla (HVF) neredeyse yerel
  hızda çalışır. x86_64 emülasyonu kat kat yavaş olurdu.
- **Derleme OrbStack'teki Debian arm64 makinesinde** (`ankaros-build`). Repo Mac'te durur, makineye
  aynı yolla paylaşılır. Buildroot çıktısı makinenin kendi diskinde tutulur.
- **Test Mac'teki QEMU ile** (`-accel hvf`). Faz 1'de GPU hızlandırmalı grafik testi için UTM kullanılır.
- **Derleme makinesi de aarch64** olduğu için Bootlin'in hazır (x86_64 üzerinde çalışan) araç zincirleri
  kullanılamaz. Buildroot kendi araç zincirini derler. Bu yalnızca ilk derlemede zaman alır.

## Sonuçlar

- Intel Mac kullanan bir ekip üyesi için QEMU çok yavaş olur (TCG emülasyonu).
- İleride gerçek donanıma geçerken (ör. x86_64 dizüstü) yeni bir defconfig ve kart klasörü eklenecek.
