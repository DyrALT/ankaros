# 0013: Uygulamalar ayrı süreçler, ortak Ankaros.UI ve Ankaros.System modülleri

- **Durum:** Kabul edildi
- **Tarih:** 2026-10-28

## Bağlam

Faz 2'de Ayarlar kabuğun içinde bir pencereydi. Bileşenler (düğme, metin alanı, bilgi balonu) ve sistem
servislerine bağlanan C++ nesneleri (Session, Network) kabuğun ve compositor'ın içindeydi. Faz 3'te
Terminal, Ayarlar ve Paket Merkezi gelecek, paketle kurulan uygulamalar da olacak. Hepsi aynı tasarım
dilinde olmalı.

## Değerlendirilen seçenekler

- **Uygulamalar kabuğun içinde:** Daha az parça. Ama bir uygulamadaki hata bütün masaüstünü çökertir,
  paketle kurulan uygulamalar bu yolla çalışamaz.
- **Ayrı Qt uygulamaları, ortak modüller:** Her uygulama kendi sürecinde bir Wayland istemcisidir.
  Bileşenler ve servis istemcileri paylaşılan QML modüllerindedir.

## Karar

Uygulamalar ayrı süreçlerdir. İki ortak QML modülü:

- **Ankaros.UI:** Tema, yazı tipleri, simgeler ve bileşenler (Label, Icon, PushButton, InputField,
  ToggleSwitch, SegmentedControl, InfoTip, SettingLabel, Avatar...). `Ankaros.Shell` ve bütün uygulamalar
  bunu kullanır.
- **Ankaros.System:** Sistem servislerinin soketlerine bağlanan C++ nesneleri (Session, Network,
  Packages). Compositor da uygulamalar da bunları kullanır.

Pencere çerçevesini compositor çizer (karar 0007), uygulamalar yalnızca içeriği çizer.

## Sonuçlar

- Tema ve diğer görünüm ayarları süreçler arasında paylaşılır. Ayar dosyası izlenir, bir süreçteki
  değişiklik diğerlerine anında yansır.
- Mac önizlemesi her uygulamayı ayrı bir pencerede gösterebilir.
- Üçüncü taraf uygulamalar Ankaros.UI'yi kullanmak zorunda değildir. Kullanırlarsa sistemle aynı
  görünürler.
