# Ses ve D-Bus

Faz 4 · A adımı ([faz-4-plani.md](faz-4-plani.md)). Katmanlardaki uygulamalar (VS Code, tarayıcılar) sesi
buradan kullanır ([karar 0016](kararlar/0016-uyumluluk-katmanlari.md)).

```
çekirdek (SND_VIRTIO, SND_HDA_INTEL)      ses aygıtı: QEMU/UTM'de virtio-sound-pci
└── PipeWire                             sesi taşır, uygulamalar ona bağlanır
    ├── WirePlumber                      hangi aygıtın varsayılan olacağına karar verir
    └── pipewire-pulse (varsa)           PulseAudio protokolü: Electron, Chromium, Firefox
D-Bus
├── sistem veri yolu                     ankaros-init'te `dbus` servisi
└── oturum veri yolu                     her oturumda `masaustu-baslat` açar
```

## Süreçler

- **Sistem D-Bus'ı:** `/etc/ankaros/services/dbus.service`. `ankaros-session`'dan önce başlar, çökerse yeniden
  başlatılır.
- **Oturum:** `ankaros-session` compositor yerine `/usr/libexec/ankaros/masaustu-baslat` çalıştırır
  ([system/scripts/masaustu-baslat](../system/scripts/masaustu-baslat)). Betik kullanıcının kimliğiyle:
  1. `$XDG_RUNTIME_DIR/bus` adresinde oturum D-Bus'ını (`dbus-daemon --session`) açar ve
     `DBUS_SESSION_BUS_ADDRESS`'i ayarlar,
  2. `pipewire`, `wireplumber` ve varsa `pipewire-pulse`'ı başlatır,
  3. compositor'ı başlatır (`exec` değil: çıkınca ses servislerini durdurması gerekir).

  Ses servisleri compositor'la aynı süreç grubundadır: oturum kapanınca `ankaros-session` hepsine birden
  sinyal gönderir. Compositor çökerse betik onları kendisi durdurur, yeniden açılan oturum temiz başlar.
  Eski soketler ve kilitler her başlangıçta silinir.
- **Giriş ekranı** (`--login`) ses altyapısı başlatmaz.
- Ses altyapısı başlamazsa (paket yok, aygıt yok) masaüstü yine açılır, ses simgesi görünmez.

## Arayüz

`Ankaros.System.Audio` (`audio.cpp`) varsayılan çıkışı `wpctl` ile okur ve değiştirir:

| Özellik | Anlamı |
|---|---|
| `available` | Varsayılan çıkış var mı. Yoksa alt çubukta simge, Hızlı ayarlarda kaydırıcı görünmez. |
| `volume` | 0-100. Yazınca `wpctl set-volume -l 1.0` çalışır (yazılımsal yükseltme yok). |
| `muted` | Sessiz durumu. `toggleMuted()` değiştirir. |

İki saniyede bir yeniden okunur, başka bir yerde yapılan değişiklik de yansır. Kullanıcı değer yazdıysa o sırada başlamış eski okuma
sonucu atılır. `wpctl` başlamazsa (PipeWire geç hazır olabilir) 60 saniyeye kadar artan aralıkla yeniden denenir,
5 saniyede dönmeyen `wpctl` öldürülür. `wpctl` çıktısının çözümlenmesi
Qt'siz [wpctl.h](../shell/qml/Ankaros/System/wpctl.h)'dedir, birim testi `wpctl_test`.
Mac önizlemesinde sahte `Audio.qml` kullanılır (aygıt varmış gibi).

Alt çubuktaki ses simgesi Windows 11 gibi kademelidir (`Audio.icon`, `wpctl.h` içindeki `iconName`):
sessizde ve ses 0'dayken hoparlör ve çarpı, 1-33 bir, 34-66 iki, 67 ve üstü üç dalga.
Alt çubukta ses simgesi, Hızlı ayarlarda sessiz düğmesi ve kaydırıcı vardır. Tasarım
Figma'dan değil, mevcut bileşenlerin diliyle yapıldı; Figma'daki hali sonradan işlenir.

## Sanal makineler

Üç betik de `virtio-sound-pci` ekler (QEMU 8.2 ve üstü):

| Betik | Ses sürücüsü |
|---|---|
| `run-qemu.sh` | Mac'te `coreaudio`, başka yerde `none`. `ANKAROS_AUDIO` ile değişir (`pa`, `pipewire`, `alsa`). |
| `run-qemu.ps1` | `dsound`. `-Audio none` ile kapanır. |
| `qemu-test.py` | `none`: CI'da ses çıkmaz, aygıtın görünmesi yeter. |
| `utm-olustur.sh` | `coreaudio` |

## Denemek

Seri konsolda (ya da Terminal'de):

```sh
wpctl status                              # Audio altında bir Sink (Built-in Audio) görünmeli
wpctl get-volume @DEFAULT_AUDIO_SINK@     # Volume: 1.00
wpctl set-volume @DEFAULT_AUDIO_SINK@ 0.3 # Hızlı ayarlardaki kaydırıcı %30'a inmeli
aplay -l                                  # çekirdek aygıtı görüyor mu
```

Aygıt `aplay -l`'de görünmüyorsa çekirdek ses desteği (`linux.fragment`) ya da sanal makinenin ses aygıtı
sorunludur; görünüp `wpctl status`'ta yoksa `/var/log/ankaros-compositor.log`'a bakın (PipeWire ve WirePlumber
çıktısı oraya düşer).

Günlükte görülen ve zararsız uyarılar: RTKit (gerçek zamanlı öncelik servisi yok), logind modülü, Bluetooth,
V4L2 ve libcamera eklentileri yok.
