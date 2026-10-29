# 0017: Kurulan uygulamalar ve katmanlar ayrı bir veri diskinde

- **Durum:** Kabul edildi
- **Tarih:** 2026-10-29

## Bağlam

Faz 4'te (karar 0016) uyumluluk katmanları ve onlara kurulan uygulamalar diske yazılacak. Debian katmanı
açılınca 250-300 MB, VS Code bağımlılıklarıyla 400 MB'tan fazla. Bugün tek bir disk var: Buildroot'un
ürettiği 768 MB'lık kök dosya sistemi. Yaklaşık 570 MB'ı boş, ama asıl sorun şu: sistem imajla
güncellenir (`utm-olustur.sh --guncelle`, ileride kurulum programı), güncelleme kök dosya sistemini baştan
yazar ve kullanıcının kurduğu her şey silinir.

## Değerlendirilen seçenekler

- **Kök dosya sistemini büyütmek:** Yer sorununu çözer, güncellemede silinme sorununu çözmez.
- **Kök dosya sisteminde ayrı bir bölüm:** Tek disk kalır, ama bölümlü bir imaj gerekir ve bugünkü
  "imajı diske kopyala" güncellemesi bölümleri bozmadan yapılamaz.
- **İkinci bir sanal disk:** Sistem diski imajın kendisi olarak kalır, veriler ayrı bir diskte durur.
  Sanal makinelerde kolay, gerçek donanımda iki bölüme karşılık gelir.

## Karar

Kurulan uygulamalar ve katmanlar ikinci bir diskte, **veri diskinde** tutulur:

- Etiketi `ankaros-veri`, dosya sistemi ext4. `/var/lib/ankaros` olarak bağlanır.
- Açılışta `veri` servisi diski bulur. Diskte hiçbir dosya sistemi yoksa (yeni, boş disk) biçimlendirir.
  Başka bir dosya sistemi ya da başka bir etiket varsa **dokunmaz**, günlüğe yazar.
- Veri diski yoksa sistem yine açılır, `/var/lib/ankaros` kök dosya sisteminde kalır (CI, eski sanal
  makineler).
- QEMU'da (`run-qemu.sh`) imaj klasöründe `veri.qcow2`, UTM'de sanal makinenin klasöründe 16 GB'lık,
  yer kapladıkça büyüyen bir disk. `utm-olustur.sh --guncelle` sistem diskini değiştirir, veri diskine
  dokunmaz.

## Sonuçlar

- Sistem güncellense de kurulan uygulamalar ve katmanlar korunur.
- Gerçek donanımda (Faz 6) kurulum programı aynı etiketle ikinci bir bölüm oluşturur.
- Ev klasörü (`/home`) şimdilik kök dosya sisteminde kalır, ileride veri diskine taşınabilir.
- İmaja e2fsprogs eklenir (`mkfs.ext4`, `e2fsck`). Busybox'ın `mke2fs`'i yalnızca ext2 biçimlendirir,
  günlüksüz bir dosya sistemi elektrik kesintisinde bozulabilir.
