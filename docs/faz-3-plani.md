# Faz 3 planı: uygulamalar (`v0.4.0`)

> **Değişti (2026-10-29):** E (kendi paket biçimimiz) ve F (Paket Merkezi) adımları yapılmadı. Kullanıcı
> uygulamaların kendi sitelerinden indirdiği `.deb` ve `.rpm` paketlerini çalıştırabilmeli. Bunun yerine
> Faz 4'te uyumluluk katmanları geliyor: [karar 0016](kararlar/0016-uyumluluk-katmanlari.md),
> [faz-4-plani.md](faz-4-plani.md). Aşağıdaki "Uygulama paketi" bölümü tarihçe olarak duruyor.
>
> **Eklendi (2026-10-29):** `v0.4.0`'dan önce masaüstü cilalanıyor: pencere yönetimi, alt çubuk ve
> başlat paneli, Ayarlar'a Klavye ve dil sayfası. Yeni E–H adımları ve [Masaüstü cilası](#masaüstü-cilası)
> bölümü.

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

Masaüstü cilası (eski E ve F'nin yerine):

| PR | İçerik |
|---|---|
| **E** `design` | Figma: Alt+Tab, pencere yaslama önizlemesi, hızlı ayarlar paneli, takvim, "Tüm uygulamalar" görünümü, Ayarlar › Klavye ve dil. **Onayın alınır.** |
| **F** `feat(compositor)` | Pencere yönetimi: kenardan ve köşeden boyutlandırma, yaslama, Alt+Tab, açılma ve kapanma animasyonları, klavye kısayolları |
| **G** `feat(shell)` | Alt çubuk ve başlat paneli: tüm uygulamalar, son kullanılanlar, Ayarlar sayfalarında arama, takvim, hızlı ayarlar, sahte simgelerin kalkması |
| **H** `feat(apps)` | Ayarlar › Klavye ve dil: klavye düzeni seçimi, anında uygulanır, giriş ekranında da geçerli |
| **I** `chore` | `v0.4.0` |

A, B ile aynı anda yürüyebilir (tasarım onayını beklemez). C ve D tasarım onayından sonra başlar.
F, G ve H, E'nin onayından sonra başlar ve birbirinden bağımsızdır.

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

## Masaüstü cilası

### Pencere yönetimi (F)

- **Boyutlandırma:** Pencere kenarlarından ve köşelerinden sürüklenerek. İmleç kenarda boyutlandırma
  okuna döner. Uygulamanın en küçük boyutu (`xdg_toplevel.min_size`) korunur.
- **Yaslama:** Pencere ekranın sol ya da sağ kenarına sürüklenince yarım ekrana, üst kenara sürüklenince
  tam ekrana yaslanır. Bırakmadan önce yerini gösteren yarı saydam bir önizleme çıkar. Yaslanmış pencere
  sürüklenince eski boyutuna döner.
- **Alt+Tab:** Açık pencerelerin küçük görüntüleri ortada bir şeritte, son kullanılandan başlayarak.
  Alt bırakılınca seçilen pencere öne gelir.
- **Animasyonlar:** Açılma, kapanma, küçültme (alt çubuktaki simgesine doğru) ve büyütme. Kısa
  (150–200 ms) ve sade.
- **Kısayollar:**

| Kısayol | İş |
|---|---|
| Süper | Başlat panelini aç ve kapat |
| Süper+D | Masaüstünü göster |
| Süper+←, → | Pencereyi sola ya da sağa yasla |
| Süper+↑, ↓ | Büyüt, eski boyutuna döndür ya da küçült |
| Alt+Tab, Alt+Shift+Tab | Pencereler arasında geçiş |
| Alt+F4 | Pencereyi kapat |
| Süper+L | Kilitle (var) |

### Alt çubuk ve başlat paneli (G)

- Komutu olmayan uygulamalar listeden kalkar.
- **Tüm uygulamalar:** Başlat panelindeki "Tümü" düğmesi alfabetik tam listeyi açar.
- **Son kullanılanlar:** Boş kalan "Önerilenler" yerine son açılan uygulamalar.
- **Arama:** Uygulamaların yanında Ayarlar sayfalarını da bulur ("koyu tema" → Görünüm).
- **Takvim:** Saate tıklanınca ay görünümü açılır.
- **Hızlı ayarlar:** Sağdaki simgelere tıklanınca açılan panel: ağ, tema, Ayarlar'a kısayol, oturumu
  kapat. Ses ve bildirim Faz 4'te eklenir.
- Gerçek karşılığı olmayan simgeler (ses, bildirim, pil) kalkar. Pil yalnızca sistemde pil varsa görünür.

### Klavye ve dil (H)

- Klavye düzeni: Türkçe Q, Türkçe F, İngilizce (ABD). Seçim anında uygulanır.
- Seçim oturum ayarlarına yazılır, giriş ekranında ve kilit ekranında da aynı düzen kullanılır.
- Sistem dili şimdilik yalnızca Türkçe, sayfada yalnızca gösterilir.

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
