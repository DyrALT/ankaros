# 0011: Kablolu ağ için kendi servisimiz

- **Durum:** Kabul edildi
- **Tarih:** 2026-10-28

## Bağlam

Faz 1'de ağı Buildroot'un `S40network` betiği DHCP ile açıyordu. Ağın durumu masaüstünden görülemiyor,
ayarları da değiştirilemiyordu. Faz 2'de Ayarlar'da bir Ağ sayfası olacak, alt çubuktaki ağ simgesi de
gerçek durumu gösterecek. VM'de yalnızca kablolu ağ var, Wi-Fi ancak gerçek donanımda denenebilir.

## Değerlendirilen seçenekler

- **NetworkManager:** Wi-Fi, VPN, her şey hazır. Ama büyüktür, D-Bus ve GLib getirir.
- **ConnMan:** Daha hafif, Wi-Fi de hazır. Yine de D-Bus ister.
- **Kendi küçük servisimiz:** Kablolu ağ için yeterli: DHCP için BusyBox `udhcpc`, statik IP için
  `ip`, durum için `/sys/class/net`. Wi-Fi gelince `wpa_supplicant` ile genişletilir.

## Karar

`ankaros-network` (C++20, Qt'siz). Ayar `/etc/ankaros/network.conf`'tadır: `dhcp` ya da `static`.
Durum ve ayar değişikliği `/run/ankaros/network.sock` üzerinden yapılır, durum değişince bağlı
istemcilere bildirilir. Ayarı yalnızca root ve etkin oturumun kullanıcısı değiştirebilir.

## Sonuçlar

- Wi-Fi geldiğinde (Faz 4) servisin genişletilmesi ya da ConnMan'e geçilmesi yeniden değerlendirilir.
  Soket protokolü masaüstünü iki durumda da aynı tutar.
- Tek bir arayüz (`eth0`) desteklenir, birden çok kablolu arayüz kapsam dışı.
