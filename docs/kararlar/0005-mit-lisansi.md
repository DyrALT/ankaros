# 0005: Proje lisansı olarak MIT

- **Durum:** Kabul edildi
- **Tarih:** 2026-10-28

## Bağlam

ankaros herkese açık bir repoda geliştiriliyor ve bir lisansa ihtiyaç duyuyor. Sistem, farklı lisanslı
bileşenlerin üzerine kurulu: Linux çekirdeği ve Buildroot GPL-2.0, Qt 6 LGPL-3.0/GPL, glibc LGPL-2.1,
BusyBox GPL-2.0.

## Değerlendirilen seçenekler

- **GPL-3.0:** ankaros'tan türetilen her şey de açık kaynak kalmak zorunda olur. Koruyucu, ama kodumuzu
  başka projelerde kullanmak isteyenler için kısıtlayıcı.
- **MIT:** En serbest seçenek. Kodumuz kapalı projelerde bile kullanılabilir, tek şart telif ve lisans
  metninin korunması.

## Karar

MIT. Lisans **sadece ankaros ekibinin yazdığı kod ve belgeler** için geçerlidir: `external/`, `shell/`,
`compositor/`, `scripts/`, `docs/` ve ileride eklenecek kendi bileşenlerimiz.

## Sonuçlar

- Üçüncü taraf bileşenler kendi lisanslarıyla kalır. `buildroot/` submodule'ü GPL-2.0'dır ve MIT
  kapsamına girmez. Derlenen imaj, içindeki her paketin kendi lisansına tabidir.
- **İmaj dağıtırken** GPL ve LGPL bileşenlerin kaynak kodunu sunmamız gerekir. Buildroot bunun için
  `make legal-info` hedefini sunuyor: tüm paketlerin lisanslarını ve kaynak kodlarını tek bir klasörde
  toplar. Faz 4'te (dağıtım) ISO ile birlikte yayınlanmalı.
- Qt LGPL-3.0 olduğu için kabuk ve compositor Qt'ye **dinamik olarak** bağlanmalıdır (imajda Qt paylaşılan
  kütüphane olarak bulunur). Böylece kullanıcı Qt'yi değiştirebilir ve LGPL koşulları sağlanır. Kabuğun
  kendi `Ankaros.Shell` modülünün statik derlenmesi buna engel değildir, o bizim kodumuz.
- Katkıda bulunanlar, katkılarının MIT lisansıyla yayınlanmasını kabul etmiş sayılır (bkz. CONTRIBUTING.md).
