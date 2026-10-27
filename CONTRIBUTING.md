# ankaros'a katkı rehberi

Bu belge ekipçe nasıl çalıştığımızı anlatır: dallar, commit mesajları, kod stili, inceleme ve belgeleme.
Kurulum için [docs/gelistirme-ortami.md](docs/gelistirme-ortami.md) belgesine bakın.

## Temel kurallar

1. **`main` her zaman derlenir ve açılır.** `main`'e doğrudan commit atılmaz, her değişiklik bir dal ve
   Pull Request (PR) üzerinden gelir.
2. **Belgelenmemiş değişiklik bitmiş sayılmaz.** Davranışı, kurulumu ya da mimariyi değiştiren her PR
   ilgili belgeyi de günceller (ayrıntılar aşağıda).
3. **Küçük ve odaklı PR'lar.** Tek bir PR tek bir iş yapar. İncelemesi 15 dakikayı aşacak bir PR
   muhtemelen bölünmelidir.

## Dal (branch) stratejisi

`main` tek uzun ömürlü daldır. İş dalları `main`'den açılır ve PR ile geri birleşir (squash merge).

| Önek | Ne için | Örnek |
|---|---|---|
| `feat/` | Yeni özellik | `feat/shell-panel-saat` |
| `fix/` | Hata düzeltme | `fix/compositor-cokme-cikista` |
| `docs/` | Sadece belge | `docs/mimari-acilis-sirasi` |
| `build/` | Buildroot, CI, derleme betikleri | `build/qt6-paketleri` |
| `refactor/` | Davranışı değiştirmeyen yeniden düzenleme | `refactor/shell-tema-ayir` |
| `chore/` | Bakım, bağımlılık güncelleme | `chore/buildroot-2026.02.4` |

Dal adları küçük harf, kelimeler tire ile ayrılır, Türkçe karakter kullanılmaz.

## Commit mesajları

[Conventional Commits](https://www.conventionalcommits.org/tr/v1.0.0/) biçimini kullanıyoruz.
Tür ve kapsam İngilizce, açıklama Türkçe:

```
<tür>(<kapsam>): <kısa açıklama, küçük harfle, noktasız>

<isteğe bağlı gövde: ne değişti ve NEDEN, 72 sütunda satır kır>
```

- **Türler:** `feat`, `fix`, `docs`, `build`, `ci`, `refactor`, `perf`, `test`, `style`, `chore`
- **Kapsamlar:** `shell`, `compositor`, `system`, `buildroot`, `kernel`, `scripts`, `docs`, `ci`
- Açıklama emir kipinde yazılır: "ekle", "düzelt" ("eklendi", "düzeltildi" değil).

Örnekler:

```
feat(shell): panele saat ve tarih bileşeni ekle
fix(buildroot): eudev için eksik çekirdek seçeneğini aç
docs(mimari): açılış sırasını belgele
```

Geriye uyumsuz bir değişiklikte türden sonra `!` eklenir ve gövdeye `BREAKING CHANGE:` satırı yazılır.

## Pull Request süreci

1. Dalınızı güncel `main`'den açın: `git switch main && git pull && git switch -c feat/...`
2. Commit'lerinizi atın, `main`'deki değişiklikleri almak için `git rebase main` kullanın.
3. PR açın. PR şablonundaki kontrol listesini doldurun.
4. En az **bir ekip arkadaşının onayı** ve **CI'ın yeşil olması** gerekir.
5. Birleştirme yöntemi **squash merge**. PR başlığı Conventional Commits biçiminde olmalı, çünkü
   `main`'deki commit mesajı o olur.
6. Birleştikten sonra dalı silin.

## Kod stili

| Dil | Araç | Ayar dosyası |
|---|---|---|
| C++20 | `clang-format`, `clang-tidy`, `clazy` | [.clang-format](.clang-format), [.clang-tidy](.clang-tidy) |
| QML | `qmlformat`, `qmllint` | [.qmlformat.ini](.qmlformat.ini) |
| Kabuk betikleri | POSIX `sh`, `shellcheck` | |
| Hepsi | EditorConfig | [.editorconfig](.editorconfig) |

### C++ kuralları

- Sahiplik açık olmalı: ya Qt parent'ı (`new Foo(this)`) ya da `std::unique_ptr`. Çıplak `delete` yok.
- Sınıflar `CamelCase`, fonksiyon ve değişkenler `camelBack`, özel üyeler `m_` önekli.
- Ağır iş ve iş mantığı C++'ta yapılır, QML'e `Q_PROPERTY` / `Q_INVOKABLE` ile açılır.
- Hata ayıklama derlemeleri ASan ve UBSan ile test edilir.

### QML kuralları

- **QML sadece görünümdür.** JavaScript'i kısa binding'ler ve küçük olay işleyicileriyle sınırlı tutun.
  Döngü, ağ, dosya ya da hesaplama içeren her şey C++'a gider.
- Renk, boşluk, yazı tipi ve süre gibi değerler doğrudan yazılmaz, `Theme` tekil nesnesinden okunur
  (bkz. `shell/qml/Ankaros/Shell/Theme.qml`). Böylece Figma'daki tasarım değişkenleriyle birebir eşleşir.
- Her bileşen kendi dosyasında, dosya adı bileşen adıyla aynı (`Panel.qml` → `Panel`).
- QML modülleri `qt_add_qml_module` ile tanımlanır ve önceden derlenir (`qmlsc`).

## Belgeleme

Belgeler Türkçe yazılır ve [docs/](docs/) altında durur. Şu durumlarda belge güncellemek zorunludur:

| Değişiklik | Güncellenecek yer |
|---|---|
| Kurulum veya derleme adımları | [docs/gelistirme-ortami.md](docs/gelistirme-ortami.md), [README.md](README.md) |
| Yeni bileşen, katman ya da açılış sırası değişikliği | [docs/mimari.md](docs/mimari.md) |
| Geri dönmesi zor bir teknik karar (dil, kütüphane, mimari) | Yeni bir karar kaydı: [docs/kararlar/](docs/kararlar/) |
| Kullanıcının fark edeceği her değişiklik | [CHANGELOG.md](CHANGELOG.md), `[Yayınlanmamış]` bölümü |
| Faz tamamlanması ya da plan değişikliği | [docs/yol-haritasi.md](docs/yol-haritasi.md) |

Kodun içinde ise **neden**i anlatan yorumlar yazılır. Kodun ne yaptığını tekrar eden yorumlar yazılmaz.
Herkese açık C++ sınıfları ve QML bileşenleri dosya başında kısa bir açıklama taşır.

## Sürümler

[Semantic Versioning](https://semver.org/lang/tr/) kullanıyoruz. 1.0'dan önce her faz bir ara sürümdür:
Faz 0 → `v0.1.0`, Faz 1 → `v0.2.0` gibi. Sürümler `main` üzerinde `vX.Y.Z` etiketiyle işaretlenir ve
CHANGELOG'daki `[Yayınlanmamış]` bölümü o sürümün başlığına dönüştürülür.

## Buildroot'u güncellemek

Buildroot bir git submodule'üdür ve bir LTS sürümüne sabitlenmiştir. Güncellemek için ayrı bir
`chore/buildroot-<sürüm>` dalı açın:

```bash
cd buildroot && git fetch --tags && git checkout 2026.02.4 && cd ..
git add buildroot && git commit -m "chore(buildroot): 2026.02.4 sürümüne güncelle"
```

Bu PR'da imajın temiz bir derlemeden sonra açıldığını doğrulayın.

## Lisans

ankaros'a gönderdiğiniz katkılar projenin [MIT lisansıyla](LICENSE) yayınlanır. Başka bir projeden kod
alıyorsanız lisansının MIT ile uyumlu olduğundan emin olun ve kaynağını PR açıklamasında belirtin. GPL
lisanslı kodu `shell/`, `compositor/` gibi kendi bileşenlerimize kopyalamayın.
