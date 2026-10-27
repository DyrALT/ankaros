# 0002: Tek dil olarak C++20, arayüz için QML

- **Durum:** Kabul edildi
- **Tarih:** 2026-10-28

## Bağlam

Masaüstü deneyimi projenin merkezinde. Ekip QML ile çok iyi arayüzler çıkarabiliyor. Ekip ayrıca
sistem genelinde **tek bir derlenen dil** kullanmak istiyor.

## Değerlendirilen seçenekler

- **Rust + Smithay + iced/Slint:** Bellek güvenliği derleyici tarafından sağlanır, ama QML'den vazgeçmek
  gerekir. Rust arayüz kütüphaneleri genç ve API'leri sık değişiyor.
- **Rust + QML (cxx-qt köprüsü):** Hem güvenlik hem QML, ama arayüz ile compositor arasında sürekli bir
  köprü katmanı, iki derleme sistemi (CMake + Cargo) ve az örnek kod.
- **C++ + QML:** QML'in doğal eşi. Qt Wayland Compositor'ın API'si doğrudan C++/QML. Tek derleme sistemi
  (CMake). Bellek güvenliği bizim sorumluluğumuzda.

## Karar

C++20 + Qt 6 + QML. Compositor Qt Wayland Compositor ile, kabuk QML ile, sistem araçları da C++ ile yazılır.

## Sonuçlar

- Bellek hatalarına karşı alışkanlıklar zorunlu: Qt parent sahipliği veya `std::unique_ptr`, çıplak
  `delete` yok, CI'da ASan/UBSan, `clang-tidy` ve `clazy`.
- QML'de sadece görünüm olur. Ağır mantık JavaScript'e değil C++'a gider, QML önceden derlenir (`qmlsc`).
- Qt 6 imaja onlarca MB ekler ve ilk derlemesi saatler sürebilir. Kabul edilebilir bir bedel.
- Qt LGPLv3/GPL lisanslıdır. ankaros'un lisansı seçilirken bu dikkate alınmalı.
