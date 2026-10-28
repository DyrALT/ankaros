# Faz 3 planı: uygulamalar (`v0.4.0`)

> **Değişti (2026-10-29):** E (kendi paket biçimimiz) ve F (Paket Merkezi) adımları yapılmadı. Kullanıcı
> uygulamaların kendi sitelerinden indirdiği `.deb` ve `.rpm` paketlerini çalıştırabilmeli. Bunun yerine
> Faz 4'te uyumluluk katmanları geliyor: [karar 0016](kararlar/0016-uyumluluk-katmanlari.md),
> [faz-4-plani.md](faz-4-plani.md). Aşağıdaki "Uygulama paketi" bölümü tarihçe olarak duruyor.

**Hedef:**
- Uygulamalar imajı yeniden derlemeden kurulup kaldırılabilir. Paket herhangi bir dilde yazılmış bir
  programı ve sistemde olmayan kütüphanelerini taşıyabilir.
- ankaros'un ilk kendi uygulamaları gelir: **Terminal**, **Ayarlar** ve **Paket Merkezi**. Üçü de
  masaüstüyle aynı tasarım dilindedir.

## Mimari

```
ankaros-compositor            masaüstü (Ankaros.Shell), pencereler
├── Terminal                  ayrı süreç, Wayland istemcisi
├── Ayarlar                   ayrı süreç, Wayland istemcisi
├── Paket Merkezi             ayrı süreç, Wayland istemcisi
└── paketle kurulan uygulamalar (/opt/ankaros/apps)

QML modülleri (her uygulama kullanır)
├── Ankaros.UI                tema, düğme, metin alanı, anahtar, bilgi balonu, simgeler...
└── Ankaros.System            oturum, ağ, paketler: sistem servislerinin soketlerine bağlanan C++ nesneleri

Sistem servisleri
└── ankaros-packages (root)   paketleri indirir, imzasını denetler, kurar, kaldırır
    └── ank                   terminal arayüzü
```

İlgili kararlar:

- [0012: Kendi uygulama paketimiz](kararlar/0012-uygulama-paketleri.md)
- [0013: Uygulamalar ayrı süreçler, ortak Ankaros.UI ve Ankaros.System modülleri](kararlar/0013-uygulama-mimarisi.md)
- [0014: Terminal için libvterm](kararlar/0014-terminal-libvterm.md)

## Adımlar

Her adım ayrı bir PR'dır. PR'lar üst üste dizilmez.

| PR | İçerik |
|---|---|
| **A** `refactor` | Ortak bileşenler `Ankaros.Shell`'den `Ankaros.UI` modülüne taşınır. Compositor'daki `Session`, `Network` ve `SocketRequest` `Ankaros.System` modülüne taşınır. Tema ayarı süreçler arasında anında paylaşılır: Ayarlar'da tema değişince masaüstü de değişir. Görünür bir değişiklik yok. |
| **B** `design` | Figma: Terminal, Ayarlar uygulaması (yeni sayfalar: Duvar kağıdı, Alt çubuk, Hakkında), Paket Merkezi. **Onayın alınır.** |
| **C** `feat(apps)` | Ayarlar uygulaması: kabuktaki Ayarlar penceresi ayrı bir uygulama olur, boş sayfalar doldurulur |
| **D** `feat(apps)` | Terminal: libvterm ile terminal öykünmesi, sekmeler, kopyala-yapıştır, tema ve yazı tipi ayarı. `weston-terminal` kalkar. |
| ~~**E**~~ `feat(system)` | Paket biçimi (`.ankpkg`), `ankaros-packages` servisi, `ank` komutu, imzalı paket deposu, örnek paket ve paketleri üreten CI hattı |
| ~~**F**~~ `feat(apps)` | Paket Merkezi: arama, kurma, kaldırma, güncelleme |
| **G** `chore` | `v0.4.0` |

A, B ile aynı anda yürüyebilir (tasarım onayını beklemez). C, D ve F tasarım onayından sonra başlar.

## Uygulama paketi

Bir `.ankpkg` dosyası zstd ile sıkıştırılmış bir tar arşividir:

```
manifest              ad = değer satırları (aşağıda)
files/                /opt/ankaros/apps/<id>/ altına olduğu gibi kurulur
  bin/hesap-makinesi
  lib/                isteğe bağlı: sistemde olmayan kütüphaneler
  share/simge.png
```

```ini
id = org.ankaros.hesap-makinesi
name = Hesap Makinesi
version = 1.0.0
summary = Dört işlem ve bilimsel hesap
exec = bin/hesap-makinesi
icon = share/simge.png
app-id = org.ankaros.hesap-makinesi      # Wayland app_id: açık penceresi alt çubukta bununla eşleşir
requires = qt6                           # sistemden beklenen çalışma ortamı (qt6, yalnızca glibc...)
```

- **Kurulum:** Paket `/opt/ankaros/apps/<id>/` altına kurulur. Masaüstü bu klasörü izler, yeni uygulama
  başlat panelinde hemen görünür. Uygulama `files/lib` varsa o klasörü `LD_LIBRARY_PATH`'e ekleyerek
  başlatılır.
- **Diller:** Paket her dilde yazılmış bir programı taşıyabilir. Sistemde Qt 6, Mesa, Wayland ve glibc
  hazırdır. Başka kütüphaneler pakete konur. Yalnızca Wayland'li uygulamalar çalışır, XWayland yok.
- **Depo:** Statik dosyalardan oluşur, GitHub'da tutulur. Paket listesi (`index`), her paketin adını,
  sürümünü, boyutunu ve SHA-256 özetini taşır. Liste ed25519 ile imzalanır, açık anahtar imajdadır.
  İmzası ya da özeti tutmayan paket kurulmaz.
- **Servis:** `ankaros-packages` (root, C++, Qt'siz) indirme (libcurl), imza (libsodium), açma (libarchive)
  işlerini yapar. `ank` komutu ve Paket Merkezi onun soketine bağlanır. Kurma ve kaldırma yetkisi
  `netdev` gibi ayrı bir grupta olur (`apps`).
- **Komutlar:** `ank ara`, `ank kur`, `ank kaldır`, `ank güncelle`, `ank liste`, `ank bilgi`.
- **Sistem uygulamaları:** Terminal, Ayarlar ve Paket Merkezi paket değildir, imajın parçasıdır
  (`/etc/ankaros/apps.json`). Paket yöneticisi bozulsa da açılmaları gerekir.

## Ayarlar uygulaması

Kabuğun içindeki Ayarlar penceresi ayrı bir uygulama olur. Görünüm, Ağ ve Kullanıcı sayfaları taşınır,
boş sayfalar doldurulur:

| Sayfa | İçerik |
|---|---|
| Görünüm | (var) tema, pencere düğmeleri |
| Duvar kağıdı | Hazır duvar kağıtlarından seçim, açık ve koyu temada ayrı |
| Alt çubuk | Alt çubuğa sabitlenmiş uygulamalar, saat biçimi |
| Ağ | (var) |
| Kullanıcı | (var) |
| Hakkında | Sürüm, çekirdek, bellek, disk, açılış süresi |

Bildirimler sayfası Faz 4'e kalır, çünkü henüz bildirim sistemi yok.

## Terminal

- libvterm ile VT220/xterm öykünmesi, metin Qt Quick sahnesinde GPU ile çizilir
- Sekmeler, kopyala-yapıştır (pano compositor üzerinden diğer uygulamalarla paylaşılır), kaydırma geçmişi
- Yazı tipi: JetBrains Mono (SIL OFL, imaja eklenir), boyutu ayarlanabilir
- Renkler temayla uyumlu, açık ve koyu temada ayrı

## Test yöntemi

- **Birim testleri:**
  - Paket manifesti, depo listesi ve imza denetimi (geçerli, değiştirilmiş, imzasız)
  - Kurulum ve kaldırmada dosya listesi
  - Terminalin satır ve ekran tamponu
- **QEMU:** Her uygulama `qemu-test.py`'nin tıklama ve tuş eylemleriyle açılıp denenir. Paket deposu
  testte yerel bir HTTP sunucusundan verilir, internet gerekmez.
- **UTM:** Gerçek GPU ile görünüm, özellikle Terminal'in metin çizimi

## Kapsam dışı

- Flatpak (ileride ikinci bir kaynak olarak, Faz 4 civarında yeniden değerlendirilecek)
- Sistemin kendisinin paketle güncellenmesi (imaj olarak kalıyor)
- Dosya yöneticisi, metin düzenleyici ve diğer uygulamalar
- Uygulamaların birbirinden yalıtılması (sandbox)

## Riskler

| Risk | Önlem |
|---|---|
| Kütüphanesini taşıyan paketler sistemdeki glibc'den yeni bir glibc isterse çalışmaz | Paketler ankaros'un Buildroot SDK'sıyla üretilir, `requires` alanıyla çalışma ortamı belirtilir |
| İmza anahtarının kaybolması ya da sızması | Özel anahtar yalnızca CI'ın gizli değişkenlerinde, anahtar değiştirme yolu belgelenir |
| Terminalde metin çizimi yavaş olursa | Glif önbelleği, yalnızca değişen satırların yeniden çizilmesi, UTM'de ölçüm |
| Ayarlar ayrı sürece geçince masaüstü ayar değişikliğini geç görür | Ayar dosyası izlenir (A adımında), değişiklik anında uygulanır |
