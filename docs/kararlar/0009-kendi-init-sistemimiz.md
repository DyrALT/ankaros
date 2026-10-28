# 0009: Kendi init sistemimiz

- **Durum:** Kabul edildi
- **Tarih:** 2026-10-28

## Bağlam

Faz 0 ve 1'de PID 1 BusyBox init'iydi. Servisler `/etc/init.d/S??*` betikleriyle sırayla başlıyordu.
Bu düzenin üç eksiği var:

- Servisler birbirini beklemek zorunda olmasa bile hepsi sırayla başlıyor.
- Çöken bir servis (örneğin compositor) yeniden başlatılmıyor.
- Hangi servisin ne zaman hazır olduğu bilinmiyor, açılış süresi ölçülemiyor.

ankaros'un hedeflerinden biri hız, yol haritasında da kendi init sistemimiz Faz 2'nin işi olarak geçiyor.

## Değerlendirilen seçenekler

- **systemd:** En yaygın ve en eksiksiz seçenek. Ama büyüktür, D-Bus ve kendi araçlarını getirir.
  Buildroot'ta glibc ile kullanılabilir, fakat sistemin geri kalanını da kendi dünyasına çeker.
- **Hazır küçük init (s6, runit) + üstünde kendi yöneticimiz:** Denenmiş bir PID 1, daha az risk. Ama
  servis tanımı, bağımlılıklar ve günlükleme iki ayrı sistem arasında bölünür, "kendi init'imiz"
  hedefinden de geri adımdır.
- **Kendi init'imiz (C++20, Qt'siz):** Tam kontrol, açılış sırası ve ölçüm tamamen bizde. PID 1'deki
  hata bütün sistemi durdurur, bu yüzden kodun küçük ve iyi denenmiş olması gerekir.

## Karar

Kendi init'imizi yazıyoruz: `ankaros-init`. Düz C++20 ve POSIX ile, Qt'siz ve istisnasız
(`-fno-exceptions`) derlenir. Servisler `/etc/ankaros/services/*.service` dosyalarıyla, `ad = değer`
satırlarından oluşan basit bir biçimle tanımlanır. Bağımlılığı olmayan servisler paralel başlar,
çöken servisler artan aralıklarla yeniden başlatılır. Denetim için `ankarosctl` komutu
`/run/ankaros/init.sock` soketine bağlanır. Ayrıntılar: [faz-2-plani.md](../faz-2-plani.md).

## Sonuçlar

- BusyBox init ve `/etc/init.d` betikleri kalkar. Buildroot paketlerinin getirdiği init betikleri
  kullanılmaz, gereken servisler (udev, seedrng, getty) bizim servis dosyalarımızla yazılır. Yeni bir
  Buildroot paketi eklendiğinde servis dosyasını da bizim yazmamız gerekir.
- Servis dosyası ayrıştırıcısı ve bağımlılık sıralaması birim testleriyle korunur, her PR'da QEMU
  açılış testi çalışır.
- Kurtarma: init açılmazsa çekirdek komut satırına `init=/bin/sh` eklenerek kabukla açılabilir.
- cgroups, soket etkinleştirme ve zamanlayıcılar kapsam dışı. Gerekirse sonradan eklenir.
