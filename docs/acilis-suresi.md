# Açılış süresi

Faz 2'de açılış süresi için bir hedef konmadı. Her adımı ölçülüyor ve raporlanıyor. İyileştirme, ölçümler
neyin zaman aldığını gösterdikten sonra yapılacak.

## Nasıl ölçülür

```bash
ankarosctl boot                                    # ankaros'ta, seri konsolda
./scripts/qemu-test.py --expect-desktop --wait 60 --screenshot masaustu.png   # Mac'ten
```

```text
ADIM                      BAŞLADI     HAZIR     SÜRE
çekirdek                     0.000     0.050     0.050
getty                        0.100     0.100     0.000
seedrng                      0.100     0.110     0.010
udev                         0.100     0.158     0.058
ankaros-network              0.158     0.158     0.000
ankaros-session              0.159     0.159     0.000
◆ masaüstü                             1.286

Toplam: çekirdek 0.08 sn, servisler 0.16 sn, masaüstü 1.29 sn
```

Raporun satırları:

- **Zamanlar:** Çekirdeğin saati (`CLOCK_BOOTTIME`) başladığından beri geçen saniye. QEMU'nun çekirdeği
  yüklemesi ve çekirdeğin saati başlatmadan önceki çok erken açılışı bu süreye girmez.
- **çekirdek:** init'in başladığı an.
- **Servisler:** Her servisin başladığı ve hazır olduğu an. Sürekli çalışan servisler başladıkları anda,
  tek seferlik servisler (`oneshot`) işleri bitince hazır sayılır ([init.md](init.md)).
- **◆ işaretleri:** Bir programın init'e bildirdiği an.
  - `masaüstü` ve `giriş ekranı`: compositor'ın ekrana ilk kareyi çizdiği an (`compositor/Main.qml`,
    `Session.markBoot`).
  - Başka programlar da `ankarosctl mark AD` ile işaret ekleyebilir. Her adın yalnızca ilk zamanı tutulur.

`qemu-test.py --expect-desktop`, masaüstü göründükten sonra raporu kendi çıktısına yazar. CI'da imaj iş
akışının özet sayfasına da eklenir.

## Ölçümler

2026-10-28, `v0.3.0` öncesi, MacBook Pro (M2 Pro), QEMU 10, imaj `ankaros_qemu_aarch64_defconfig`:

| Ortam | Çekirdek | Servisler hazır | Masaüstü (ilk kare) |
|---|---|---|---|
| QEMU, hvf (donanım hızlandırma), 3 ölçüm | 0,04–0,10 sn | 0,13–0,24 sn | 1,04–1,18 sn |
| QEMU, tcg (yazılımsal taklit, CI'daki gibi) | 0,82 sn | 1,78 sn | 17,90 sn |
| UTM 4.7.5, hvf, virgl (Mac'in GPU'su) | 0,29 sn | 0,38 sn | 1,33 sn |

QEMU'da ekranı (2B sanal GPU) Mesa'nın softpipe sürücüsü çizer, UTM'de virgl ile Mac'in GPU'su
([utm.md](utm.md)). UTM'de çekirdek daha geç başlıyor (3B sanal GPU'nun hazırlanması), masaüstünün ilk
karesi QEMU'dakiyle hemen hemen aynı sürede geliyor.

## Gözlemler

- **Açılışın çoğu compositor'da geçiyor.** Masaüstüne kadarki sürenin yaklaşık %85'i compositor'ın
  başlayıp ilk kareyi çizmesi: Qt'nin yüklenmesi, QML'in oluşturulması, yazı tiplerinin okunması ve
  softpipe ile ilk çizim. init ve bütün servisler 0,2 saniyenin altında hazır.
- **udev en uzun servis** (hvf'de 0,06 sn, tcg'de 0,9 sn). Oturum yöneticisi, ekran kartı ve girdi
  aygıtlarının izinleri udev'den geldiği için onu bekler.
- **tcg'de her şey 10-15 kat yavaş.** CI'daki ölçümler mutlak değer olarak değil, sürümler arasındaki
  değişimi görmek için kullanılmalı.

## İleride bakılabilecekler

Hedef konmadığı için yapılmadı, ölçümler bunların nerede fark yaratacağını gösteriyor:

- **Compositor'ın açılışı:**
  - Qt'nin hangi aşamada ne kadar beklediği (`QT_LOGGING_RULES`, `QSG_INFO`).
  - Yazı tipi önbelleğinin imajda hazır gelmesi.
  - İlk karede gerekmeyen QML bileşenlerinin sonradan yüklenmesi (`Loader`).
- **Oturum yöneticisini udev'i beklemeden başlatmak:** Ekran kartının izni udev'den önce ayarlanabilirse
  yaklaşık 0,06 sn (tcg'de 0,9 sn) kazanılır.
- **Çekirdek yapılandırmasını küçültmek:** QEMU'da kullanılmayan sürücüleri çıkarmak.
