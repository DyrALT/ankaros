# Değişiklik günlüğü

Bu dosya [Keep a Changelog](https://keepachangelog.com/tr-TR/1.1.0/) biçimini, sürümler
[Semantic Versioning](https://semver.org/lang/tr/) kurallarını izler.

## [Yayınlanmamış]

### Değişti

- Ayarlar ayrı bir uygulama (`ankaros-settings`): kabuğun içindeki pencere yerine kendi sürecinde açılıyor,
  Görünüm'de yapılan değişiklik masaüstüne anında yansıyor. Alt çubuktaki ağ simgesi Ayarlar'ı Ağ
  sayfasıyla açıyor.
- Ortak bileşenler `Ankaros.UI`, oturum ve ağ istemcileri `Ankaros.System` modülüne taşındı (karar 0013).
  Görünüm ayarları dosya izlenerek süreçler arasında anında paylaşılıyor (karar 0015).
- CI'da kabuk Qt 6.9 ile açılıp ekran görüntüleri alınıyor (duman testi)

### Düzeltildi

- Başlat panelinde ve alt çubukta henüz yazılmamış uygulamalar görünüyordu, tıklayınca bir şey olmuyordu.
  Artık yalnızca açılabilen uygulamalar var. Karşılığı olmayan zil ve ses simgeleri kalktı, pil simgesi
  yalnızca sistemde pil varsa görünüyor.
- Bir pencerenin içinde sürükleyince arkasında kalan pencere taşınabiliyordu (ör. terminalde metin seçerken)
- Uygulama pencereleri bazen bir sonraki tıklamaya kadar eski görüntüde kalıyordu (ör. Terminal'de komut
  çıktısı, Ayarlar'da tıklamanın gecikmesi). Compositor "kare çizildi" bildirimini geç gönderiyordu, Qt
  uygulamaları da 100 ms'de alamayınca çizmeyi bırakıyordu. Bildirim artık hemen gönderiliyor,
  uygulamalara 1 saniye süre tanınıyor.
- Simgeler UTM'de (virgl, Mac'in GPU'su) kesik kesik çiziliyordu. Qt Shapes'in CurveRenderer'ı yerine
  klasik çizici kullanılıyor, kenarlar süper örneklemeyle yumuşatılıyor.

### Eklendi

- Ayarlar › Klavye ve dil: Türkçe Q, Türkçe F ya da İngilizce (ABD) klavye düzeni. Seçim uygulamalara ve
  masaüstüne hemen uygulanıyor, giriş ve kilit ekranında da kullanılıyor.
- Başlat paneli ve alt çubuk:
  - "Tümü" bütün uygulamaları alfabetik listeliyor, "Önerilenler" yerine son kullanılan uygulamalar var.
  - Arama Ayarlar'daki ayarları da buluyor ("koyu" → Görünüm). Ayarlar açıksa istenen sayfaya geçiyor.
    Oklarla seçilip Enter ile açılıyor.
  - Saate tıklayınca takvim, durum simgelerine tıklayınca hızlı ayarlar açılıyor (ağ, koyu tema, kilitle,
    Ayarlar, güç menüsü).
- Pencere yönetimi (`docs/pencere-yonetimi.md`): kenarlardan ve köşelerden boyutlandırma, ekranın
  yarısına ya da tamamına yaslama (önizlemeli), Alt+Tab ile pencere geçişi, açılma, kapanma ve küçültme
  animasyonları. Kısayollar: Süper (başlat paneli), Süper+D, Süper+oklar, Alt+F4.
- Terminal (`ankaros-terminal`, `docs/terminal.md`): ankaros'un kendi terminali. libvterm ile xterm
  öykünmesi, sekmeler, kopyala-yapıştır, kaydırma geçmişi, JetBrains Mono yazı tipi, temayla uyumlu
  renkler. `weston-terminal` ve Weston imajdan çıktı.
- Retina ekran desteği: 2560 piksel ve daha geniş ekranlarda arayüz 2 kat ölçekle, keskin çiziliyor.
  `utm-olustur.sh` sanal makineyi Retina modunda kuruyor.
- Kabuk istemi renkli ve klasörü gösteriyor (`kullanıcı@makine:~$`), `ls` renkli
- Ayarlar'ın yeni sayfaları:
  - **Duvar kağıdı:** dört palet (Ankara Taşı, Bozkır, Kehribar, Gece), her birinin açık ve koyu tonu var.
    İstenirse kilit ve giriş ekranında da kullanılır.
  - **Alt çubuk:** sabitlenecek uygulamalar ve sıraları (sürükleyerek), 24 ya da 12 saat biçimi, tarihi
    gösterme
  - **Hakkında:** sürüm, çekirdek, işlemci, bellek, disk, ekran kartı ve açılış süresi, "Bilgileri kopyala"
- Giriş ekranı kullanıcının temasını ve duvar kağıdını kullanıyor (oturum yöneticisi aktarır)
- `utm-olustur.sh --guncelle`: var olan UTM sanal makinesinin diskini ve kernel'ini yeni imajla değiştirir

## [0.3.0] - 2026-10-28

Faz 2: Sistem servisleri. ankaros kendi init sistemiyle açılıyor, masaüstü bir kullanıcı hesabıyla
çalışıyor, giriş ve kilit ekranı, kablolu ağ ayarları ve açılış süresi raporu var.

### Eklendi

- Kendi init sistemimiz `ankaros-init` (`docs/init.md`): servis dosyaları, bağımlılık sırasıyla paralel
  başlatma, çöken servisi yeniden başlatma, servis başına günlük, düzgün kapatma. Servisleri denetleyen
  `ankarosctl` komutu.
- Kullanıcı hesapları ve oturum yöneticisi `ankaros-session` (`docs/oturum.md`): masaüstü root yerine
  `ankaros` kullanıcısıyla (şifre `ankaros`) çalışıyor, geliştirme imajında otomatik giriş açık
- Giriş ekranı, kilit ekranı (Süper+L), başlat panelinde güç menüsü (Kilitle, Oturumu kapat, Yeniden
  başlat, Kapat) ve Ayarlar › Kullanıcı (otomatik giriş, şifre değiştirme)
- Kablolu ağ servisi `ankaros-network` (`docs/ag.md`): DHCP ya da statik IP, durum değişince
  abonelere bildirim. Ayarlar › Ağ ve alt çubukta gerçek durumu gösteren kablolu ağ simgesi
- Açılış süresi raporu (`docs/acilis-suresi.md`): `ankarosctl boot` çekirdeği, servisleri ve
  masaüstünün ilk karesini gösteriyor, `qemu-test.py` ve CI raporu yazdırıyor. Mac'te (QEMU, hvf)
  masaüstü yaklaşık 1,1 saniyede geliyor.
- Uygulama listesi `/etc/ankaros/apps.json`'dan okunuyor (`docs/uygulama-listesi.md`)
- Ayarlarda bilgi balonları: açıklamalar sayfaya yazılmıyor, ayar adının yanındaki ⓘ simgesinde.
  Bütün sistem için ortak tasarım ilkeleri (`docs/tasarim-akisi.md`).
- `qemu-test.py`'ye tuş, yazı, seri konsol komutu, ağ kablosu ve ara ekran görüntüsü eylemleri

### Değişti

- BusyBox init ve `/etc/init.d` betikleri kaldırıldı
- Masaüstünü init değil `ankaros-session` başlatıyor. Yeniden başlatmak için:
  `ankarosctl restart ankaros-session`
- CI'da `main`'deki imaj derlemesi yeni birleştirmelerde yarıda kesilmiyor
- `build.sh` değişen defconfig'i (ör. `git pull` sonrası) kendiliğinden yeniden uyguluyor

## [0.2.0] - 2026-10-28

Faz 1: Açılışta gelen grafik masaüstü. ankaros'un kendi Wayland compositor'ı, pencereler ve Görünüm ayarları.

### Eklendi

- Grafik yığını: Qt 6.9 (eglfs, QML, Wayland compositor), Mesa (virgl, softpipe), libinput, weston-terminal
- `ankaros-shell` paketi: kabuk ankaros'ta tam ekran çalışıyor
- Türkçe dil ayarı (tr_TR.UTF-8) ve İstanbul saat dilimi
- `ankaros-compositor`: Qt Wayland Compositor ile Wayland sunucusu ve tam ekran masaüstü. Pencereler
  ankaros çerçevesiyle açılıyor, taşınıyor, öne geliyor, küçültülüp büyütülüyor, kapanıyor. Alt
  çubuktaki göstergeler gerçek pencerelerden geliyor, Terminal alt çubuktan açılıyor.
- Kabuğa pencere çerçevesi (`WindowChrome`, `WindowTitleBar`) ve pencere katmanı
- Masaüstü açılışta kendiliğinden başlıyor (`/etc/init.d/S90ankaros-desktop`)
- Ayarlar › Görünüm: tema, pencere düğmelerinin konumu (sol/sağ) ve stili (simge/renkli daire).
  Bütün pencerelere anında uygulanıyor, `~/.config/ankaros/shell.conf`'ta saklanıyor.
- `run-qemu.sh --gui` ve ekran görüntüsü alan otomatik test betiği `scripts/qemu-test.py`
- CI'da açılış testi: her imaj derlemesinden sonra imaj QEMU'da açılır, masaüstünün ekrana geldiği
  denetlenir, ekran görüntüsü yüklenir
- UTM'de GPU hızlandırmalı sanal makineyi tek komutla kuran `scripts/utm-olustur.sh` ve rehberi (`docs/utm.md`)

### Değişti

- CI'daki imaj derlemesi ayrı iş akışına (`imaj.yml`) taşındı. Yalnızca imajı etkileyen dosyalar
  değişince çalışıyor, PR'lar onu beklemiyor.

## [0.1.0] - 2026-10-28

Faz 0: QEMU'da açılan en küçük ankaros sistemi ve Mac'te çalışan masaüstü kabuğu.

### Eklendi

- Buildroot 2026.02 LTS tabanlı derleme sistemi ve `ankaros_qemu_aarch64_defconfig`
- Linux 6.18, glibc, BusyBox ve eudev ile en küçük sistem
- `scripts/build.sh`, `scripts/progress.sh`, `scripts/run-qemu.sh`
- Masaüstü kabuğu (`Ankaros.Shell`), Figma'daki "B · Ankara Taşı" tasarımıyla: buzlu cam alt çubuk
  (başlat, arama, uygulamalar, durum, saat), aramalı başlat paneli (sabitlenmiş uygulamalar, önerilenler),
  açık/koyu tema, gömülü Inter yazı tipi, Qt Shapes ile çizilen simgeler
- Kabuğu Mac'te gösteren önizleme uygulaması ve `scripts/shell-preview.sh`
- Kod stili ayarları (clang-format, clang-tidy, qmlformat, EditorConfig)
- MIT lisansı
- Ekip belgeleri: katkı rehberi, mimari, geliştirme ortamı, yol haritası, karar kayıtları

### Düzeltildi

- `run-qemu.sh`: OrbStack klasöründeki imaj açılırken çıkan disk kilidi hatası

[Yayınlanmamış]: https://github.com/DyrALT/ankaros/compare/v0.3.0...HEAD
[0.3.0]: https://github.com/DyrALT/ankaros/releases/tag/v0.3.0
[0.2.0]: https://github.com/DyrALT/ankaros/releases/tag/v0.2.0
[0.1.0]: https://github.com/DyrALT/ankaros/releases/tag/v0.1.0
