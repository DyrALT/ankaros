> [!WARNING]
> ankaros geliştirme aşamasındadır ve güvenlik için sertleştirilmemiştir. İmajları yalnızca sanal makinede
> deneyin, önemli verilerinizi koymayın.

## İmajlar

Sanal makine (QEMU, UTM) imajlarıdır, gerçek donanıma kurulmaz. Mac ve ARM bilgisayarlarda `aarch64`, öteki
bilgisayarlarda `x86_64` kullanın. Arşivi açıp deponun bu sürümündeki betikle başlatın:

```sh
tar xzf ankaros-@TAG@-aarch64.tar.gz
IMAGES=$PWD/ankaros-@TAG@-aarch64 ./scripts/run-qemu.sh --gui
```

UTM için: [docs/utm.md](https://github.com/@REPO@/blob/@TAG@/docs/utm.md). Veri diski imajda yoktur, ilk açılışta
boş olarak oluşturulur. İndirdiğiniz dosyayı `sha256sum -c SHA256SUMS --ignore-missing` ile denetleyebilirsiniz.

## Değişiklikler
