# 0004: C kütüphanesi olarak glibc

- **Durum:** Kabul edildi
- **Tarih:** 2026-10-28

## Bağlam

Buildroot üç C kütüphanesi sunuyor: uClibc-ng (varsayılan), musl ve glibc. Araç zinciri ilk derlemede
seçilir ve sonradan değiştirmek tüm sistemi sıfırdan derlemeyi gerektirir.

## Karar

glibc. Qt 6, Mesa ve Wayland yığını en çok glibc ile test ediliyor. uClibc-ng ve musl daha küçük, ama
Faz 1'de karşılaşacağımız uyumluluk sorunlarını şimdiden önlemek istedik.

## Sonuçlar

- İmaj birkaç MB daha büyük olur.
- Hız hedefi açısından fark ihmal edilebilir. Açılış süresini asıl belirleyen şeyler çekirdek
  yapılandırması ve init sırası.
