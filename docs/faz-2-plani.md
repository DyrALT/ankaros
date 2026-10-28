# Faz 2 planı: sistem servisleri (`v0.3.0`)

**Hedef:** ankaros kendi init sistemiyle açılır. Masaüstü root olarak değil, normal bir kullanıcı
hesabıyla çalışır. Açılışta Ankara Taşı tasarımında bir giriş ekranı gelir, istenirse otomatik giriş
yapılır. Ekran kilitlenebilir, oturum kapatılabilir, sistem masaüstünden kapatılıp yeniden
başlatılabilir. Ayarlar'daki Ağ sayfasında kablolu ağ görülür ve DHCP ya da elle IP olarak ayarlanır.
Açılış süresi her adımıyla ölçülür ve raporlanır.

## Mimari

```
Linux
└── ankaros-init (PID 1)            dosya sistemlerini bağlar, servisleri başlatır ve izler, kapatır
    ├── udev, seedrng, getty         sistem servisleri (/etc/ankaros/services/*.service)
    ├── ankaros-network              kablolu ağ: DHCP ya da statik IP, durum bilgisi
    └── ankaros-session              oturum yöneticisi (root)
        ├── giriş ekranı              ankaros-compositor --login, "ankaros-login" kullanıcısı olarak
        └── kullanıcı oturumu         ankaros-compositor, giriş yapan kullanıcı olarak
```

- Servisler birbirleriyle ve masaüstüyle `/run/ankaros/` altındaki Unix soketleri üzerinden konuşur.
  Protokol satır tabanlı metindir (`komut argümanlar` → `ok ...` ya da `hata ...`). JSON kütüphanesi
  ya da D-Bus gerekmez.
- `ankaros-init`, `ankaros-session` ve `ankaros-network` Qt kullanmaz. Düz C++20 ve POSIX ile
  yazılırlar, açılışta Qt'nin yüklenmesini beklemezler. Kodları `system/` klasöründedir, tek bir
  Buildroot paketi (`ankaros-system`) olarak kurulurlar.
- Compositor bu soketlere Qt'nin `QLocalSocket`'i ile bağlanır. QML'e `Session` ve `Network` tekil
  nesneleri olarak açılır. Mac önizlemesinde bunların sahte sürümleri kullanılır.

İlgili kararlar:

- [0009: Kendi init sistemimiz](kararlar/0009-kendi-init-sistemimiz.md)
- [0010: Oturum yönetimi ve giriş ekranı](kararlar/0010-oturum-yonetimi.md)
- [0011: Kablolu ağ için kendi servisimiz](kararlar/0011-ag-servisi.md)

## Adımlar

Her adım ayrı bir PR'dır. PR'lar üst üste dizilmez, bir sonraki adıma geçmeden önce birleştirilir.

| PR | İçerik |
|---|---|
| **A** `feat(compositor)` | Uygulama listesi `/etc/ankaros/apps.json`'dan okunur: ad, komut, Wayland app_id'leri, alt çubuğa sabitlenmiş mi. Compositor'daki sabit liste kalkar. |
| **B** `feat(init)` | `ankaros-init`: dosya sistemleri, servis dosyaları, bağımlılık sırasıyla paralel başlatma, çöken servisi yeniden başlatma, servis günlükleri, kapatma ve yeniden başlatma. `ankarosctl` komutu. BusyBox init ve `/etc/init.d` betikleri kalkar. Ayrıştırıcı ve bağımlılık sıralaması için birim testleri. |
| **C** `design` | Figma: giriş ekranı, kilit ekranı, başlat panelindeki güç menüsü, Ayarlar'daki Kullanıcı ve Ağ sayfaları. **Onayın alınır.** |
| **D** `feat(session)` | `ankaros` kullanıcısı, `ankaros-session`, giriş ve kilit ekranları, otomatik giriş, oturumu kapatma, güç menüsü, Ayarlar › Kullanıcı (otomatik giriş, şifre değiştirme) |
| **E** `feat(network)` | `ankaros-network`, Ayarlar › Ağ, alt çubuktaki ağ simgesinin gerçek durumu göstermesi |
| **F** `perf` | Açılış süresi ölçümü: `ankarosctl boot` her adımın süresini gösterir. `qemu-test.py` ve CI raporu yazdırır. Ölçüm sonuçları belgelenir, sınır konmaz. |
| **G** `chore` | `v0.3.0` |

B'nin sonunda sistem yine root olarak masaüstüne açılır, yalnızca init değişmiş olur. Oturum ve ağ
B'nin üstüne kurulur.

## ankaros-init

Servis dosyası örneği (`/etc/ankaros/services/udev.service`):

```ini
# Aygıt yöneticisi
description = Aygıt yöneticisi (eudev)
type = oneshot             # oneshot: çalışıp biter | simple: sürekli çalışır (varsayılan)
exec = /sbin/udevd --daemon
exec = /sbin/udevadm trigger --action=add
exec = /sbin/udevadm settle
after = seedrng            # önce bunlar hazır olmalı
```

| Alan | Anlamı |
|---|---|
| `exec` | Çalıştırılacak komut. `oneshot` serviste birden çok satır sırayla çalışır. |
| `type` | `simple`: başladığında hazır sayılır, sürekli çalışır. `oneshot`: başarıyla bittiğinde hazır sayılır. |
| `after` | Bu servisler hazır olmadan başlatılmaz. Bağımlılığı olmayan servisler aynı anda başlar. |
| `restart` | `on-failure` (simple için varsayılan): çökerse artan aralıklarla yeniden başlar. `no`: başlamaz. |
| `user` | Servisin çalışacağı kullanıcı. Varsayılan root. |
| `environment` | `AD=değer`, birden çok satır olabilir |

- **Açılış:** `/proc`, `/sys`, `/dev`, `/dev/pts`, `/run`, `/tmp` bağlanır. Kök dosya sistemi yazılabilir
  olarak yeniden bağlanır, makine adı ayarlanır, servisler başlatılır.
- **Günlükler:** Her servisin çıktısı `/var/log/<servis>.log` dosyasına yazılır. init'in kendi günlüğü
  hem konsola hem `/var/log/ankaros-init.log`'a gider, her satırda açılıştan itibaren geçen süre yazar.
- **Denetim:** `ankarosctl status | start | stop | restart <servis>`, `ankarosctl poweroff | reboot`,
  `ankarosctl boot`. BusyBox'ın `poweroff` ve `reboot` komutları da çalışmaya devam eder.
- **Kapatma:** Servisler başlatılma sırasının tersiyle durdurulur. Kalan süreçlere önce SIGTERM, sonra
  SIGKILL gönderilir. Diskler senkronlanır, salt okunur yapılır, makine kapatılır.
- **Çökme koruması:** PID 1 çökerse çekirdek panik verir. init'in kodu küçük tutulur, istisna (exception)
  kullanılmaz, başlatılan programlar hiçbir zaman init'in kendi sürecinde çalışmaz.

## Oturum

- **Hesaplar:** `ankaros` kullanıcısı (şifre `ankaros`, `video`, `input`, `audio` gruplarında) ve
  giriş ekranı için şifresiz, oturum açamayan bir sistem kullanıcısı (`ankaros-login`). root şifresiz
  kalır ve yalnızca seri konsoldan girilebilir.
- **Akış:** `ankaros-session` önce `/etc/ankaros/session.conf`'a bakar. Otomatik giriş açıksa doğrudan o
  kullanıcının oturumunu başlatır. Kapalıysa giriş ekranını açar.
  - Giriş ekranı kullanıcı adını ve şifreyi `ankaros-session`'a gönderir. Şifreyi `/etc/shadow` ile
    `ankaros-session` karşılaştırır, giriş ekranı şifre dosyasını göremez.
  - Giriş başarılı olunca giriş ekranı kapanır. `ankaros-session` kullanıcının
    `XDG_RUNTIME_DIR`'ını hazırlar ve compositor'ı o kullanıcı olarak başlatır.
  - Oturum kapatılınca giriş ekranı yeniden gelir.
  - Hatalı denemeden sonra cevap gecikir, art arda denemeyle şifre bulmak yavaşlar.
- **Kilit ekranı:** Kullanıcının compositor'ı içinde çalışır. Şifre yine `ankaros-session` tarafından
  doğrulanır. `ankaros-session` soketteki sürecin kimliğine bakar (`SO_PEERCRED`), bir kullanıcı yalnızca
  kendi şifresini doğrulatabilir.
- **Güç menüsü** (başlat paneli): Kilitle, Oturumu kapat, Yeniden başlat, Kapat
- **Ayarlar › Kullanıcı:** Otomatik giriş açma/kapama, şifre değiştirme. VM imajında otomatik giriş
  varsayılan olarak açıktır.

## Ağ

- `ankaros-network`, `/etc/ankaros/network.conf`'taki ayara göre `eth0`'ı açar: `dhcp` (varsayılan) ya
  da `static` (adres, ağ geçidi, DNS). DHCP için BusyBox'ın `udhcpc`'si kullanılır.
- Bağlantı durumu (kablo takılı mı, IP adresi, ağ geçidi, DNS) soketten okunur. Durum değişince bağlı
  istemcilere bildirilir.
- Ayar değişikliğini yalnızca root ve etkin oturumun kullanıcısı yapabilir.
- Wi-Fi Faz 4'te, gerçek donanımla birlikte.

## Açılış süresi

Hedef konmaz, ölçülüp raporlanır. `ankarosctl boot` örneği:

```text
çekirdek            0.62 sn
init → udev         0.35 sn
init → ağ (DHCP)    1.10 sn
oturum → masaüstü   1.84 sn   (compositor'ın ilk karesi)
toplam              2.46 sn
```

Ölçüm Mac'te (hvf) ve CI'da (tcg) alınır, sonuçlar `docs/acilis-suresi.md`'ye yazılır.

## Test yöntemi

- **Birim testleri:** Servis dosyası ayrıştırıcı, bağımlılık sıralaması, `session.conf` ve
  `network.conf` okuyucuları. CI'da yeni bir "Sistem servisleri" işiyle derlenip çalıştırılır.
- **QEMU:** `qemu-test.py --expect-desktop` her adımda çalışır. Giriş ekranı, kilit ekranı ve oturum
  kapatma için ekran görüntüleri alınır, klavye girdisi QMP ile gönderilir.
- **UTM:** Günlük kullanım ve gerçek hız.

## Kapsam dışı

- Wi-Fi, Bluetooth, ses
- Çok kullanıcılı giriş ekranı, hesap ekleyip silme
- Uyku ve hazırda bekletme
- Servislerin kaynak sınırları (cgroups)

## Riskler

| Risk | Önlem |
|---|---|
| init'teki bir hata sistemi açılmaz hale getirir | Birim testleri, her PR'da QEMU açılış testi. Gerekirse çekirdeğe `init=/bin/sh` verip kurtarma kabuğuyla açılabilir (belgelenir). |
| Root olmayan compositor ekrana (DRM) ve girdi aygıtlarına erişemez | Kullanıcı `video` ve `input` gruplarında olur. Ekranı ilk açan süreç DRM yöneticisi olur, oturum değişirken ekranı önceki compositor bırakmış olmalıdır. |
| Şifre doğrulamada güvenlik açığı | Şifreyi yalnızca root olan `ankaros-session` görür. `crypt()` ile karşılaştırılır, hatalı denemeler geciktirilir. |
| Giriş ekranından oturuma geçerken ekran titrer | Kabul edilir. İleride tek compositor içinde geçiş düşünülebilir. |
