# ankaros-init

ankaros'un init sistemi. Çekirdeğin başlattığı ilk süreçtir (PID 1). Görevleri:

- dosya sistemlerini bağlamak,
- servisleri bağımlılık sırasıyla başlatmak, çöken servisleri yeniden başlatmak,
- sistemi düzgünce kapatmak.

C++20 ile yazılmıştır, Qt kullanmaz. Kod `system/` klasöründedir. Neden kendi init'imizi yazdığımız:
[karar 0009](kararlar/0009-kendi-init-sistemimiz.md).

## Açılış

1. `/proc`, `/sys`, `/dev`, `/dev/pts`, `/dev/shm`, `/run` ve `/tmp` bağlanır. Kök dosya sistemi
   yazılabilir olarak yeniden bağlanır, makine adı `/etc/hostname`'den ayarlanır.
2. `/etc/ankaros/services/*.service` dosyaları okunur. Hatalı bir dosya günlüğe yazılıp atlanır, açılışı
   durdurmaz. Hiç servis okunamazsa seri konsolda giriş istemi açılır.
3. Bağımlılığı olmayan servisler hemen, olanlar bağımlılıkları hazır olunca başlar.
4. init olayları bekler: biten süreçler, kapatma sinyalleri, `ankarosctl` komutları.

## Servis dosyaları

Her servis `/etc/ankaros/services/<ad>.service` dosyasıdır. Satırlar `ad = değer` biçimindedir. `#` satır
başında ya da önünde boşluk varsa yorumdur.

```ini
# Aygıt yöneticisi
description = Aygıt yöneticisi (eudev)
type = oneshot
exec = udevd --daemon
exec = udevadm trigger --type=devices --action=add
exec = udevadm settle --timeout=30
```

| Alan | Değer | Anlamı |
|---|---|---|
| `description` | metin | `ankarosctl status`'ta görünen açıklama |
| `exec` | komut | Çalıştırılacak komut. Tek ve çift tırnak desteklenir, kabuk özellikleri (değişken, `;`, `>`) yoktur. Gerekirse `sh -c '...'` yazılır. |
| `type` | `simple` (varsayılan) | Sürekli çalışır, başlatıldığı anda hazır sayılır. Tek `exec` olabilir. |
| | `oneshot` | `exec` satırları sırayla çalışır. Hepsi başarıyla bitince servis hazır sayılır. Biri başarısız olursa geri kalanı çalışmaz. |
| `after` | servis adları | Bu servisler hazır olmadan (ya da başarısız olmadan) başlatılmaz. Boşlukla ayrılır. |
| `restart` | `on-failure` (simple için varsayılan) | Sıfırdan farklı kodla çıkarsa ya da bir sinyalle sonlanırsa yeniden başlatılır |
| | `always` | Her bitişte yeniden başlatılır (ör. getty) |
| | `no` | Yeniden başlatılmaz. oneshot için tek seçenek budur. |
| `user` | kullanıcı adı | Servisin çalışacağı kullanıcı. Varsayılan root. `HOME`, `USER`, `SHELL` buna göre ayarlanır. |
| `environment` | `AD=değer` | Ortam değişkeni. Birden çok satır olabilir. |

- Servis adı dosya adından gelir: küçük harf, rakam, `-`, `_` ve `.` kullanılabilir.
- Bilinmeyen bir alan hata sayılır, böylece yazım hataları fark edilir.
- Bilinmeyen bir servise ya da döngü oluşturan bir bağımlılık günlüğe yazılır ve yok sayılır.
- Çöken servis art arda 0,5, 1, 2, 4, 8, 16 ve 30 saniye bekleyerek yeniden başlatılır. 10 saniyeden
  uzun çalıştıktan sonra çökerse sayaç sıfırlanır.
- Her servis kendi oturumunda çalışır (`setsid`). Durdurulurken bütün süreç grubuna SIGTERM ve SIGHUP
  gönderilir, 5 saniyede durmazsa SIGKILL.

Repodaki servisler:

| Servis | Kaynak | Ne yapar |
|---|---|---|
| `syslogd`, `klogd` | `system/services/` | Sistem ve çekirdek günlüğü, `/var/log/messages` |
| `seedrng` | `system/services/` | Rastgele sayı üretecini besler |
| `veri` | `system/services/`, `system/scripts/veri-diski` | Veri diskini `/var/lib/ankaros`'a bağlar, yeni ve boşsa biçimlendirir ([karar 0017](kararlar/0017-veri-diski.md)) |
| `udev` | `system/services/` | Aygıt yöneticisi |
| `ankaros-network` | `system/services/` | Kablolu ağ: DHCP ya da statik IP ([ag.md](ag.md)) |
| `getty` | `system/services/` | Seri konsolda giriş istemi |
| `ankaros-session` | `system/services/` | Oturum yöneticisi: giriş ekranı ya da masaüstü ([oturum.md](oturum.md)) |

Bir Buildroot paketi kendi servisini getiriyorsa servis dosyası o paketin `.mk` dosyasında
`/etc/ankaros/services/` altına kurulur (örnek: `ankaros-system.mk`).

## Günlükler

| Dosya | İçerik |
|---|---|
| `/var/log/ankaros-init.log` | init'in kendi günlüğü. Aynı satırlar seri konsola da yazılır. |
| `/var/log/<servis>.log` | Servisin standart çıktısı ve hata çıktısı. Her başlatmada başa komut satırı yazılır. |

Satırların başındaki sayı, çekirdek saatinin başlamasından beri geçen süredir (saniye). `/var/log`
RAM'deki `/tmp`'dedir, yeniden başlatınca silinir.

## ankarosctl

```bash
ankarosctl status                 # servislerin durumu
ankarosctl restart ankaros-session
ankarosctl stop network
ankarosctl start network
ankarosctl boot                   # açılış süresi raporu (docs/acilis-suresi.md)
ankarosctl poweroff               # ya da BusyBox'ın poweroff, reboot, halt komutları
```

`status` ve `boot` herkes tarafından, diğer komutlar yalnızca root tarafından kullanılabilir.
`ankarosctl` `/run/ankaros/init.sock` soketine bağlanır. Protokol satır tabanlıdır, ayrıntısı
`system/common/protocol.h`'de.

## Kapatma

`ankarosctl poweroff`/`reboot`, BusyBox'ın `poweroff`/`reboot`/`halt` komutları (PID 1'e SIGUSR2, SIGTERM,
SIGUSR1 gönderirler) ve Ctrl-Alt-Del (SIGINT) kapatmayı başlatır:

1. Servisler başlatılma sırasının tersiyle durdurulur, en fazla 5 saniye beklenir.
2. Kalan bütün süreçlere SIGTERM, 2 saniye sonra SIGKILL gönderilir.
3. Diskler senkronlanır, kök dosya sistemi salt okunur yapılır, makine kapatılır ya da yeniden başlar.

## Geliştirme

```bash
# Birim testleri (Mac'te de çalışır)
cmake -S system -B build/system -G Ninja && cmake --build build/system && ctest --test-dir build/system

# İmaja almak
./scripts/build.sh ankaros-system-rebuild all
```

Birim testleri servis dosyası ayrıştırıcısını, komut bölmeyi ve bağımlılık sıralamasını denetler. CI'da
"Sistem servisleri" işi bunları Linux'ta derleyip çalıştırır. init'in kendisi QEMU açılış testiyle
denenir.

**Kurtarma:** init açılmıyorsa çekirdek komut satırına `init=/bin/sh` eklenir, sistem doğrudan kabukla
açılır. Örneğin `run-qemu.sh`'teki `-append` satırına eklenebilir.
