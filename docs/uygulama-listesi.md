# Uygulama listesi

Başlat panelindeki ve alt çubuktaki uygulamalar `/etc/ankaros/apps.json` dosyasından okunur. Repodaki
kaynağı [shell/qml/Ankaros/System/apps.json](../shell/qml/Ankaros/System/apps.json), imaja `ankaros-shell`
paketiyle kurulur. Okuyan sınıf Ankaros.System'deki `AppCatalog`'dur. Dosya yoksa (Mac önizlemesi) modüle
gömülü kopyası kullanılır.

```json
{
    "apps": [
        {
            "name": "Terminal",
            "command": "weston-terminal",
            "appIds": ["org.freedesktop.weston.wayland-terminal", "weston-terminal"],
            "pinned": true
        },
        { "name": "Dosyalar", "pinned": true }
    ]
}
```

| Alan | Anlamı |
|---|---|
| `name` | Başlat panelinde ve alt çubukta görünen ad. Simgesi de bu adla seçilir (`shell/.../IconData.js`). |
| `command` | Uygulamaya tıklanınca çalıştırılan komut. Yoksa uygulama listede görünür ama açılmaz (henüz yazılmamış uygulamalar). |
| `appIds` | Uygulamanın pencerelerinin Wayland `app_id`'leri. Açık pencerenin hangi uygulamaya ait olduğu, alt çubukta hangi simgenin altında "çalışıyor" göstergesi yanacağı buna göre bulunur. |
| `pinned` | `true` ise uygulama, çalışmasa da alt çubukta durur. |

- Uygulamalar başlat panelinde dosyadaki sırayla, alt çubukta sabitlenmişler önce olmak üzere görünür.
- Listede olmayan bir uygulamanın penceresi alt çubukta `app_id`'siyle görünür. Yeni bir uygulamanın
  `app_id`'sini öğrenmenin en kolay yolu budur.
- İmajı yeniden derlemeden denemek için dosyayı sanal makinede düzenleyip compositor'ı yeniden başlatın.
  Başka bir dosya `ANKAROS_APPS` ortam değişkeniyle de verilebilir.
- Dosya okunamazsa compositor yine açılır, başlat paneli boş kalır, günlüğe uyarı yazılır.
