# Mimari karar kayıtları (ADR)

Geri dönmesi zor her teknik karar burada kısa bir kayıt olarak tutulur: bağlam, karar, sonuçlar.
Amaç, aylar sonra "bunu neden böyle yaptık?" sorusuna cevap verebilmek.

Yeni kayıt için son numarayı bir artırın ve `0000-sablon.md` dosyasını kopyalayın. Kabul edilmiş bir
kayıt değiştirilmez. Karar değişirse yeni bir kayıt yazılır ve eskisinin durumu
"Yerine geçen: 00XX" olarak güncellenir.

| No | Karar | Durum |
|---|---|---|
| [0001](0001-linux-cekirdegi-ve-buildroot.md) | Linux çekirdeği ve Buildroot | Kabul edildi |
| [0002](0002-cpp-ve-qml.md) | Tek dil olarak C++20, arayüz için QML | Kabul edildi |
| [0003](0003-aarch64-orbstack-qemu.md) | aarch64 hedefi, OrbStack ile derleme, QEMU ile test | Kabul edildi |
| [0004](0004-glibc.md) | C kütüphanesi olarak glibc | Kabul edildi |
| [0005](0005-mit-lisansi.md) | Proje lisansı olarak MIT | Kabul edildi |
| [0006](0006-qt-wayland-compositor-ve-eglfs.md) | Compositor için Qt Wayland Compositor ve eglfs | Kabul edildi |
| [0007](0007-sunucu-tarafi-pencere-kenarliklari.md) | Pencere kenarlıklarını compositor çizer, kullanıcı özelleştirebilir | Kabul edildi |
| [0008](0008-kullanici-ayarlari.md) | Kullanıcı ayarları QtCore Settings ile ~/.config/ankaros altında | Yerine geçen: 0015 |
| [0009](0009-kendi-init-sistemimiz.md) | Kendi init sistemimiz | Kabul edildi |
| [0010](0010-oturum-yonetimi.md) | Oturum yönetimi ve giriş ekranı | Kabul edildi |
| [0011](0011-ag-servisi.md) | Kablolu ağ için kendi servisimiz | Kabul edildi |
| [0012](0012-uygulama-paketleri.md) | Kendi uygulama paketimiz | Yerine geçen: 0016 |
| [0013](0013-uygulama-mimarisi.md) | Uygulamalar ayrı süreçler, ortak Ankaros.UI ve Ankaros.System modülleri | Kabul edildi |
| [0014](0014-terminal-libvterm.md) | Terminal için libvterm | Kabul edildi |
| [0015](0015-gorunum-ayarlari-surecler-arasi.md) | Görünüm ayarları süreçler arasında paylaşılır | Kabul edildi |
| [0016](0016-uyumluluk-katmanlari.md) | Başka sistemlerin uygulamaları için uyumluluk katmanları | Kabul edildi |
