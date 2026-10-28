# 0014: Terminal için libvterm

- **Durum:** Kabul edildi
- **Tarih:** 2026-10-28

## Bağlam

Faz 1'den beri terminal olarak `weston-terminal` kullanılıyor (foot, Qt 6.9 compositor'ıyla uyumsuzdu).
Görünümü ankaros'un tasarım diline uymuyor, sekmesi ve ayarı yok. Faz 3'te kendi terminalimizi yazıyoruz.
Bir terminalin en zor kısmı, VT100/xterm kaçış dizilerini doğru yorumlayan öykünücüdür.

## Değerlendirilen seçenekler

- **QTermWidget:** Qt için hazır terminal bileşeni. GPL lisanslı, MIT olan projemizle birleşince
  uygulamayı GPL yapar. Widget tabanlı, Qt Quick ile uyumsuz.
- **Öykünücüyü sıfırdan yazmak:** Tam kontrol, ama binlerce kaçış dizisinin doğru yorumlanması aylar sürer.
- **libvterm:** Neovim'in de kullandığı küçük, C ile yazılmış öykünücü. MIT lisanslı. Yalnızca ekran
  durumunu tutar, çizim bize kalır.

## Karar

libvterm. Ekran durumunu libvterm tutar, metni Qt Quick sahnesinde biz çizeriz. libvterm Buildroot'ta
hazır olmadığı için BR2_EXTERNAL ağacına kendi paketimiz olarak eklenir.

## Sonuçlar

- Metin çizimi, kaydırma geçmişi, seçme ve kopyalama bizim kodumuzdadır.
- `weston-terminal` ve Weston bağımlılığı imajdan çıkar, imaj küçülür.
