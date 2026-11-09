# Wayland protokolleri

`ankaros-compositor`'ın uygulamalara sunduğu protokoller. Sunulan listeyi görmek için Debian katmanında
`wayland-info` kullanılabilir (`wayland-utils` paketi).

## Sunulanlar

| Protokol | Sürüm | Ne için | Nereden |
|---|---|---|---|
| `wl_compositor`, `wl_subcompositor`, `wl_shm`, `wl_output` | 4, 1, 2, 2 | Temel | Qt |
| `wl_seat`, `wl_data_device_manager` | 5, 3 | Girdi, pano, sürükle-bırak | Qt'ye yama (`external/patches/qt6wayland/`) |
| `xdg_wm_base`, `zxdg_decoration_manager_v1` | 2, 1 | Pencereler ve çerçeve (karar 0007) | Qt, `xdg_wm_base` 2 için yama (xwayland-satellite ister) |
| `zxdg_output_manager_v1` | 3 | Ekranın mantıksal boyutu | Qt, ilk değerler için yama |
| `wp_viewporter` | 1 | Ölçekleme | Qt, `WaylandCompositor` kendisi sunar |
| `zwp_linux_dmabuf_v1` | 3 | GPU ile çizilen arabellekler | Qt eklentisi, `main.cpp` seçer |
| `wp_cursor_shape_manager_v1` | 1 | Uygulamanın imleç şekli | `cursorshape.cpp` |
| `xdg_activation_v1` | 1 | Pencereyi öne getirme | `xdgactivation.cpp` |
| `zwp_primary_selection_device_manager_v1` | 1 | Seçili metni orta tuşla yapıştırma | `primaryselection.cpp` |

Kendi yazdıklarımızın tanımları `wayland-protocols` paketinden gelir, Qt'nin üreteciyle derlenir
(`compositor/CMakeLists.txt`).

## Sınırlar

- **`linux-dmabuf` sürüm 3:** Mesa'nın kendisi GPU aygıtını sürüm 4'ten ya da `wl_drm`'den öğrenir. Mesa 26'da
  `wl_drm` kalktı, Qt 6.9 da sürüm 3'te. Bu yüzden Mesa kullanan uygulamalar (Terminal, Ayarlar) büyük olasılıkla yazılımla çizer
  ve `libEGL warning: failed to get driver name for fd -1` yazar (UTM'de de). Pencereleri doğru açılıyor. Sürüm 4
  için Qt'ye yama gerekir.
- **İmleç:** Uygulamanın `wl_pointer.set_cursor` ile verdiği resim çizilmez, ok görünür. Boş istek imleci gizler.
  Şekli `cursor-shape` ile söyleyen uygulamalarda doğru imleç görünür, desteklemeyenlerin kendi imleç teması
  görünmez.
- **Seri numaraları:** `primary-selection` ve `cursor-shape` istekteki olay numarasına bakmaz, yalnızca
  uygulamanın odakta olup olmadığına bakar. `xdg-activation` de bakmaz: jeton, uygulama odaktayken ve son 10 saniyede
  tuşa ya da fareye basıldıysa geçerli olur (en fazla 64 jeton, 30 saniye).
- **`zxdg_output_v1` 3:** Qt bağlanma anında `zxdg_output_v1.done` gönderiyordu, sürüm 3 istemcileri ise
  değerleri `wl_output.done` ile uygular. Yamayla (`0003-xdg-output-3-done.patch`) `wl_output.done` gönderiliyor.
  Yamasız Xwayland ekranı 0x0 sanıyordu ([xwayland.md](xwayland.md)).

## Bilerek eklenmeyenler

| Protokol | Neden |
|---|---|
| `text-input` | Sistemde giriş yöntemi yok, Türkçe karakterler `wl_keyboard` ile çalışıyor. Sunulursa Qt uygulamaları kendi dead-key işlemeyi bırakıp boş bir yönteme bağlanabilir. Giriş yöntemi gelince eklenir. |
| `fractional-scale` | Sistemde kesirli ölçek ayarı yok. Ayar gelince eklenir. |
| `idle-inhibit` | Sistemde boşta kalınca kilitlenme ya da ekran kapatma yok. O gelince eklenir. |
| `presentation-time` | Qt'de var, ama kare zamanını ekran döngüsünden bizim bildirmemiz gerekiyor. Bildirilmezse bekleyen uygulama takılabilir. |
| `pointer-constraints`, `relative-pointer` | Yalnızca oyunlar ister. |

## Deneme

Yeni protokoller iki istemcili küçük bir programla QEMU'da denendi: seçim bir istemciden ötekine aktarıldı, arka
plandaki istemcinin seçim değiştirmesi ve geçersiz jetonla öne gelmesi reddedildi, geçerli jeton pencereyi öne
getirdi, odağını kaybeden istemci eski teklifle seçimi okuyamadı, istemciler kapanınca compositor çalışmaya devam etti.

UTM'de elle denendi: Terminal ve Ayarlar açılıyor, imleç şekli değişiyor, iki `foot` penceresi arasında yapıştırma
çalışıyor. `linux-dmabuf` sunuluyor ve hatasız yükleniyor, ama onunla gerçek bir GPU arabelleği gönderen bir uygulama
henüz denenmedi. VS Code denemesi bunu da gösterecek.
