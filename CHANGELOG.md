# Değişiklik günlüğü

Bu dosya [Keep a Changelog](https://keepachangelog.com/tr-TR/1.1.0/) biçimini, sürümler
[Semantic Versioning](https://semver.org/lang/tr/) kurallarını izler.

## [Yayınlanmamış]

### Eklendi

- Terminal'de sekmeler sürüklenerek sıralanıyor, çubuğun dışına bırakılan sekme yeni bir pencereye taşınıyor
  (içinde çalışan program kesilmiyor). Terminal birden çok pencere açabiliyor (Ctrl+Shift+N). Bir uygulamanın
  ikinci ve sonraki pencereleri farenin olduğu yerde açılıyor ([docs/terminal.md](docs/terminal.md)).
- `ank kur` ile kurulan uygulamaların kendi simgeleri (`.desktop`'taki `Icon=`) alt çubukta, başlat panelinde,
  pencere değiştiricide ve başlık çubuğunda görünüyor. Üçüncü taraf logosu olduğu gibi gösteriliyor. Simge
  katmandan güvenle okunuyor: sembolik bağlantı izlenmiyor, boyut ve içerik denetleniyor
  ([docs/katmanlar.md](docs/katmanlar.md)).

### Değişti

- CI'daki imaj derlemesi artımlı: Buildroot'un çıktısı önbellekte tutulur, yalnızca kabuk, compositor ve
  sistem paketleri yeniden derlenir (her mimari için ~50-60 dakika yerine 10 dakika kadar, tahmin). Buildroot,
  defconfig ya da `external/` değişince her şey yine baştan derlenir. PR'larda imaj yalnızca sistem katmanı
  değişince derlenir, kabuk ve compositor PR'larını CI iş akışı denetler.
- Büyütülen pencere bütün ekranı kaplıyor (tam ekran), öndeyken alt çubuk animasyonla aşağı iniyor. Fare
  ekranın en altına gelince alt çubuk geri çıkıyor ([docs/pencere-yonetimi.md](docs/pencere-yonetimi.md)).
  Ekrandan büyük açılan pencere artık büyütülmüyor, ekranın ortasında daha küçük açılıyor.

### Düzeltildi

- Üst üste duran pencerelerde öndekinin başlık çubuğuna basmak arkadaki pencereyi öne getirmiyor, öndeki
  pencere taşınabiliyor.
- Ekran kilitliyken masaüstü çökerse oturum yeniden açılmıyor, giriş ekranına dönülüyor. Kabuk kilitlendiğini
  oturum yöneticisine bildiriyor (`lock`, [docs/oturum.md](docs/oturum.md)).
- Kilit ekranı sağ ve orta tıklamayı arkadaki pencerelere geçirmiyor.
- Son görünür pencere küçültülünce klavye odağı onda kalmıyor.
- `ank katman kur` bilinmeyen katman adında duruyor, `/etc`'si bağlantı olan katman arşivini kurmuyor.
- Ayarlar › Kullanıcı'da şifre değiştirme hatası gösteriliyor, alanlar boşken Enter istek göndermiyor.
- init: durdurulan servisin süreç grubunda kalan süreçler bitene (en geç 5 sn, sonra SIGKILL) kadar servis
  yeniden başlatılmıyor. Kapanışta duramayan servisler öldürülüyor. Hiç hazır olmadan çöken bir bağımlılık
  üç denemeden sonra bağımlı servisleri bekletmiyor.
- Oturum yöneticisi kullanıcının `shell.conf`'unu onun yetkisiyle ve en fazla 64 KiB okuyor.
- Veri diski yalnızca sanal makinenin virtio disklerinde aranıyor, aynı etiketli bir USB bellek bağlanmıyor.
- WirePlumber ve pipewire-pulse PipeWire'ın soketi hazır olunca başlıyor.
- Terminal: Ctrl+[ \ ] ^ _ ve Ctrl+Boşluk kontrol karakteri gönderiyor (Ctrl+I/J/M da), küçük tekerlek
  adımları birikiyor, geçmişten atılan satırlar seçimdeyse kopyalama boş satır vermiyor, sonu gelmeyen
  başlık dizisi belleği büyütmüyor.
- Masaüstü menüsünün boşluklarına tıklamak menüyü kapatmıyor.
- `utm-olustur.sh` `qemu-img` yoksa UTM'ye dokunmadan duruyor.
- Masaüstünün çıktısı (`/var/log/ankaros-compositor.log`) oturum yöneticisinden geçiyor: dosya 4 MB'ta
  döndürülüyor, uygulamalar dosyaya doğrudan yazamıyor ([docs/oturum.md](docs/oturum.md)).
- init, güç komutu verildikten sonra hiçbir servisi başlatmıyor ya da yeniden başlatmıyor. Kapanış sürerken
  farklı bir güç isteği hata veriyor.
- `ankarosctl mark` metnindeki terminal kontrol karakterleri (C0 ve C1) ayıklanıyor.
- `ankarosctl session passwd` şifreleri `ps`'te görünmesin diye argüman olarak değil, stdin'den (terminalde
  yankısız) okuyor. Boş, sekmeli, satır sonu ya da NUL içeren şifreler kabukta, ankarosctl'de ve oturum
  yöneticisinde açık hatayla reddediliyor.
- Kilit açılınca şifre alanı ve bekleme durumu sıfırlanıyor. Kilitliyken ağ ve pil baloncukları kilit
  ekranının üstünde görünmüyor. Büyük saat görünür olunca hemen doğru saati gösteriyor.
- Ses düzeyi ayarlanırken eski bir okuma değeri geri sıçratmıyor. `wpctl` başlatılamazsa artan aralıklarla
  yeniden deneniyor, takılırsa öldürülüyor.
- Ağ servisiyle bağlantı kopunca alt çubuk "bağlı" göstermiyor. Fare ve klavye pilleri sistem pili sayılmıyor
  ([docs/ses.md](docs/ses.md), [docs/ag.md](docs/ag.md)).
- Süper+D ile küçültülen pencereler listesi bir pencere kapanınca, elle geri getirilince ya da yeni pencere
  açılınca kaybolmuyor.
- Başlat panelinde Tab ve Shift+Tab ile arama kutusundan sonuçlara ve öğelere geçiliyor, Enter ya da Boşluk
  ile açılıyor. Alt+Tab'ın arka planı tıklamayı arkadaki pencerelere geçirmiyor.
- Ayarlar › Alt çubuk, kurulu olmayan uygulamaların sabitlemesini silmiyor. Uygulama listesi yalnızca `appIds`
  ya da sabitleme değişince de güncelleniyor.
- Terminal: pencere büyüyünce geri kaydırma konumu geçersiz kalmıyor, kabuk çıkınca pty kapatılıp süreç
  arayüzü bekletmeden toplanıyor, alternatif ekranda (less, vim) Shift+PageUp ana ekranın geçmişini
  göstermiyor.
- Sürüm tek kaynaktan geliyor: `/etc/os-release` ve Ayarlar › Hakkında depo kökündeki `VERSION` dosyasını
  kullanıyor, sürüm etiketi bu dosyayla uyuşmazsa CI imaj derlemesini baştan durduruyor.
- QEMU testi açılış raporu alınamazsa başarısız oluyor. `progress.sh` paket sayısı sıfırken sıfıra bölmüyor.
  Qt Wayland yaması `wl_data_offer.finish` ve `set_actions` için protokol hatalarını denetliyor.

## [0.5.0] - 2026-11-07

### Eklendi

- Sürüm etiketi gönderilince aarch64 ve x86_64 imajları derlenip açılış testinden sonra GitHub Release'e
  ekleniyor (CONTRIBUTING.md → Sürümler).
- Masaüstü sağ tık menüsü ([docs/masaustu-menusu.md](docs/masaustu-menusu.md)): duvar kağıdı ve görünüm
  ayarları, açık/koyu tema, Terminal, ekranı kilitle. Menü satırları `Ankaros.UI`'daki ortak `MenuItem` ve
  `MenuSeparator` bileşenlerine taşındı (güç menüsü de kullanıyor). Önizlemede `--context-menu`, CI duman
  testinde bir ekran görüntüsü daha.
- Ses ve D-Bus ([docs/ses.md](docs/ses.md)): imajda PipeWire, WirePlumber ve D-Bus var, çekirdekte ses desteği
  açık. Her oturumda kullanıcının D-Bus'ı, PipeWire'ı ve WirePlumber'ı açılır. Alt çubukta ses simgesi,
  Hızlı ayarlarda sessiz düğmesi ve ses kaydırıcısı eklendi (ses aygıtı yoksa görünmezler). QEMU ve UTM
  betikleri sanal makineye `virtio-sound-pci` ekler.
- Compositor `wp_viewporter`, `zxdg_output_manager_v1`, `zwp_linux_dmabuf_v1`, `wp_cursor_shape_manager_v1`,
  `xdg_activation_v1` ve `zwp_primary_selection_device_manager_v1` sunuyor
  ([docs/wayland-protokolleri.md](docs/wayland-protokolleri.md)). Seçili metin orta tuşla yapıştırılır, uygulamalar
  imleç şeklini belirler ve odaktayken başka pencereyi öne getirebilir.
- x86_64 imajı ([karar 0018](docs/kararlar/0018-x86-64-imaji.md)): `build.sh` bilgisayarın mimarisini derler,
  `run-qemu.sh` ve `qemu-test.py` mimariyi imajdan anlar. CI x86_64'ü `main`'de derleyip açılışını denetler.
- `ank kur`, `ank kaldir` ve `ank liste` ([docs/katmanlar.md](docs/katmanlar.md)): `.deb` paketi şifre sorulduktan
  sonra Debian katmanına kurulur, uygulaması başlat panelinde görünür. İşi root olarak çalışan `ankaros-apps`
  servisi yapar.
- Compositor `wl_seat` sürüm 5 ve `wl_data_device_manager` sürüm 3 sunuyor (Qt 6.9.1'e yama,
  `external/patches/qt6wayland/`). Debian katmanındaki `foot` artık penceresini açıyor.
- Debian katmanı ([docs/katmanlar.md](docs/katmanlar.md)): `ank katman kur debian` Debian 13'ü indirip
  özetini denetleyerek veri diskine kurar, `ank katman calistir` bir komutu katmanın içinde çalıştırır.
  Kullanıcı uygulamaları root yetkisi olmadan, kullanıcı ad alanında açılır (`ankaros-katman`). İmaja
  curl, OpenSSL ve CA sertifikaları eklendi, çekirdekte kullanıcı ad alanları açıldı.
- Veri diski ([karar 0017](docs/kararlar/0017-veri-diski.md)): kurulan uygulamalar ve uyumluluk katmanları
  ikinci bir diskte (`/var/lib/ankaros`) durur, sistem güncellense de korunur. Boş disk ilk açılışta
  biçimlendirilir. `run-qemu.sh`, `qemu-test.py` ve `utm-olustur.sh` diski kendisi oluşturur.
- Windows'ta açma: `scripts/run-qemu.ps1` x86_64 imajını WHPX ile donanım hızlandırmalı açar, virgl ve veri
  diskini kendisi ayarlar ([docs/gelistirme-ortami.md](docs/gelistirme-ortami.md)). `-Width`/`-Height`
  çekirdek argümanı `ankaros.mode` ile konuğa iletilir (imajdaki sabit `kms.json` 1440x900'e kilitliyordu)

### Değişti

- Ayarlar'daki soluk "Bildirimler" öğesi kalktı (bildirim sistemi henüz yok)
- Giriş ekranının sol altındaki klavye düğmesi seçilen düzeni gösteriyor (TR, F ya da US)

### Düzeltildi

- QEMU'da (virtio-gpu) x86_64 imajının ekranı 640x480 açılıyordu, 1440x900'e ayarlandı
- Penceredeki uygulamalar (Ayarlar, Terminal) sanal GPU varken de yazılımla (softpipe) çiziyordu: Mesa
  `EGL_WL_bind_wayland_display` desteği olmadan derleniyordu, compositor `wl_drm` sunamıyor ve istemcinin EGL'si
  GPU'yu bulamıyordu. Mesa seçeneği açıldı (GPU'suz ortamda compositor'ı çökertmesin diye Mesa'ya küçük bir yama
  eklendi), uygulamalar `wayland-egl` ve `linux-dmabuf` eklentilerini yüklüyor. x86_64'te
  (QEMU, virgl) Ayarlar virgl ile çiziyor, UTM'de ve katmandaki uygulamalarla denenmedi
- Veri diski ikinci açılıştan itibaren bağlanmıyordu, kurulu katmanlar görünmüyordu
- Pencere boyutlandırılırken çerçeve fareyi geriden izliyor, fare durduktan sonra da büyümeye devam ediyordu
- İmleç pencere kenarlarında boyutlandırma imlecine dönmüyordu
- Terminal sekmesinin başlığı klasör değişince güncellenmiyordu, hep `~` kalıyordu

## [0.4.0] - 2026-10-29

### Değişti

- Ayarlar ayrı bir uygulama (`ankaros-settings`): kabuğun içindeki pencere yerine kendi sürecinde açılıyor,
  Görünüm'de yapılan değişiklik masaüstüne anında yansıyor. Alt çubuktaki ağ simgesi Ayarlar'ı Ağ
  sayfasıyla açıyor.
- Ortak bileşenler `Ankaros.UI`, oturum ve ağ istemcileri `Ankaros.System` modülüne taşındı (karar 0013).
  Görünüm ayarları dosya izlenerek süreçler arasında anında paylaşılıyor (karar 0015).
- CI'da kabuk Qt 6.9 ile açılıp ekran görüntüleri alınıyor (duman testi)

### Düzeltildi

- Alttaki bir pencerenin içine tıklamak onu öne getirmiyor ve klavye odağını vermiyordu, yalnızca başlık
  çubuğundan tıklanınca çalışıyordu
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

[Yayınlanmamış]: https://github.com/DyrALT/ankaros/compare/v0.5.0...HEAD
[0.5.0]: https://github.com/DyrALT/ankaros/releases/tag/v0.5.0
[0.4.0]: https://github.com/DyrALT/ankaros/releases/tag/v0.4.0
[0.3.0]: https://github.com/DyrALT/ankaros/releases/tag/v0.3.0
[0.2.0]: https://github.com/DyrALT/ankaros/releases/tag/v0.2.0
[0.1.0]: https://github.com/DyrALT/ankaros/releases/tag/v0.1.0
