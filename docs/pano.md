# Pano paylaşımı (UTM)

ankaros sanal makinede çalışırken Mac'te kopyalanan metin ankaros'ta yapıştırılabilir, ankaros'ta kopyalanan
metin Mac'te yapıştırılabilir. Yalnızca metin (UTF-8) paylaşılır, resim ve dosya paylaşılmaz.

```
Mac panosu ── UTM (SPICE istemcisi) ── virtio-serial: com.redhat.spice.0 ── ankaros-compositor (SpiceAgent)
                                                                                   │
                                                              Wayland panosu (wl_data_device)
                                                                                   │
                                                                     uygulamalar (Terminal, Chromium...)
```

## Nasıl çalışır

UTM ekranı SPICE ile gösterir. Pano paylaşımı açıkken sanal makineye `com.redhat.spice.0` adlı bir
virtio-serial kanalı ekler. Linux dağıtımlarında kanalın öbür ucunda `spice-vdagent` çalışır, ama o yalnızca X11
panosunu bilir. ankaros'ta aynı protokolün (spice-protocol, `vd_agent.h`) pano kısmını compositor konuşur
(`compositor/spiceagent.cpp`).

- **Mac'te kopyalanınca:** UTM `CLIPBOARD_GRAB` gönderir. Compositor metni hemen ister (`CLIPBOARD_REQUEST`),
  gelince panoyu kendisi doldurur (`QWaylandCompositor::overrideSelection`). O an öndeki uygulama da, sonradan
  odaklanan uygulamalar da bu metni yapıştırır.
- **ankaros'ta kopyalanınca:** Compositor uygulamanın panoya koyduğu içeriği okur (Qt'nin "retained selection"
  kipi, `ankaroscompositor.cpp`). Metin varsa UTM'ye `CLIPBOARD_GRAB` gönderilir. Mac'te yapıştırılınca UTM metni
  ister, compositor gönderir.
- Mac'ten gelen metin Mac'e geri bildirilmez, döngü olmaz.
- Kanal yoksa (pano paylaşımı kapalı sanal makine, gerçek bilgisayar) hiçbir şey değişmez. Giriş ekranında pano
  paylaşılmaz.

## İzinler

Kanal (`/dev/virtio-ports/com.redhat.spice.0`) `users` grubuna açıktır
(`external/board/qemu/rootfs-overlay/etc/udev/rules.d/70-ankaros-spice.rules`). Oturum açan kullanıcı bu
gruptadır, giriş ekranının kullanıcısı (`ankaros-login`) değildir. Kanalı okuyan Mac'in panosunu da okuyabilir.

## Qt yaması

`external/patches/qt6wayland/0004-pano-odak.patch`: Klavye odağı bir uygulamaya geçince Qt yalnızca başka bir
uygulamanın panosunu sunuyordu. Pano compositor'daysa (Mac'ten gelen metin ya da kopyalayan uygulama kapandıktan
sonra saklanan içerik) yalnızca o an odakta olan uygulama görüyordu.

## Yan etki

Qt'nin "retained selection" kipi açıkken compositor kopyalanan her içeriği (bütün biçimleriyle) okur. Kopyalayan
uygulama kapansa da içerik panoda kalır. Çok büyük içerik kopyalandığında (ör. büyük bir resim) bu bir miktar
bellek ve zaman alır. UTM'ye yalnızca metin gönderilir, en fazla 8 MB.

## UTM ayarı

UTM'de sanal makinenin ayarlarında **Sharing › Clipboard Sharing** açık olmalıdır (yeni sanal makinelerde
varsayılan). Kapatılırsa kanal eklenmez, ankaros'ta pano yalnızca kendi içinde çalışır.

## Deneme

QEMU'da UTM yerine küçük bir Python programı SPICE istemcisi gibi davrandı: kanal bir Unix soketine bağlandı
(`-chardev socket ... -device virtserialport,name=com.redhat.spice.0`). İstemcinin gönderdiği metin Terminal'de
Ctrl+Shift+V ile yapıştırıldı. Terminal'de seçilip Ctrl+Shift+C ile kopyalanan metin istemciye ulaştı.
