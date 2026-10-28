# 0007: Pencere kenarlıklarını compositor çizer, kullanıcı özelleştirebilir

- **Durum:** Kabul edildi
- **Tarih:** 2026-10-28

## Bağlam

Wayland'de pencerenin başlık çubuğunu ve küçült/büyüt/kapat düğmelerini ya uygulamanın kendisi çizer
(istemci tarafı, CSD) ya da compositor (sunucu tarafı, SSD). Kullanıcının düğmelerin konumunu (sağ/sol)
ve stilini (çizgi simgeler/renkli daireler) ayarlardan değiştirebilmesi isteniyor.

## Değerlendirilen seçenekler

- **İstemci tarafı (CSD):** Her uygulama kendi stilinde çizer. GTK, Qt ve foot farklı görünür, kullanıcı
  ayarı bütün uygulamalara uygulanamaz.
- **Sunucu tarafı (SSD):** Compositor, `xdg-decoration` protokolüyle uygulamalardan kenarlık çizmemelerini
  ister ve başlık çubuğunu kendisi çizer. Bütün pencereler aynı görünür, ayar her yere anında uygulanır.

## Karar

Sunucu tarafı kenarlıklar. Compositor `XdgDecorationManagerV1` ile tercih edilen modu `ServerSideDecoration`
olarak bildirir. Başlık çubuğu Ankaros.Shell modülünde bir QML bileşenidir (`WindowTitleBar`), böylece
Mac önizlemesinde de tasarlanıp denenebilir.

Başlık çubuğu iki ayarı okur (bkz. 0008):

- `windowButtonSide`: `"right"` (varsayılan) ya da `"left"`
- `windowButtonStyle`: `"icons"` (varsayılan, çizgi simgeler) ya da `"colored"` (renkli daireler)

## Sonuçlar

- `xdg-decoration` desteklemeyen ve kendi kenarlığını çizmekte ısrar eden uygulamalarda (bazı GTK
  uygulamaları) çift başlık çubuğu görülebilir. Bu uygulamalar için ileride bir istisna listesi gerekebilir.
- Başlık çubuğu tasarımı uygulamaya değil, ankaros'a aittir. Figma'da bir bileşen olarak tutulur.
