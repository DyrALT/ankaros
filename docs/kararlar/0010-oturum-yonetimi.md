# 0010: Oturum yönetimi ve giriş ekranı

- **Durum:** Kabul edildi
- **Tarih:** 2026-10-28

## Bağlam

Faz 1'de compositor root olarak çalışıyordu ve açılışta doğrudan masaüstü geliyordu. Faz 2'de
masaüstü normal bir kullanıcı hesabıyla çalışmalı, bir giriş ekranı ve kilit ekranı olmalı.
Geliştirme sırasında her açılışta şifre girmemek için otomatik giriş de seçilebilmeli.

Şifre `/etc/shadow`'da durur ve onu yalnızca root okuyabilir. Ekrana ve girdi aygıtlarına erişen
compositor'ın ise root olmaması gerekir.

## Değerlendirilen seçenekler

- **Hazır bir giriş yöneticisi (greetd, SDDM, LightDM) ve logind/seatd:** Denenmiş ve eksiksiz. Ama
  D-Bus, PAM ve kendi yapılandırmalarını getirirler. Tasarımı da kendi bileşenlerimizle yapamayız.
- **Giriş ekranı compositor'ın içinde, compositor root olarak çalışır:** En basiti. Ama kullanıcının
  bütün uygulamaları root'un compositor'ına bağlanır, compositor'daki bir hata sistemi ele geçirtir.
- **Küçük bir oturum yöneticisi (root) + iki ayrı compositor süreci:** Oturum yöneticisi şifreyi
  doğrular ve doğru kullanıcıyla süreç başlatır. Giriş ekranı yetkisiz bir sistem kullanıcısıyla,
  masaüstü giriş yapan kullanıcıyla çalışır.

## Karar

Kendi küçük oturum yöneticimiz: `ankaros-session` (C++20, Qt'siz, root). Giriş ekranı
`ankaros-compositor --login` olarak `ankaros-login` kullanıcısıyla çalışır. Kilit ekranı ise
kullanıcının compositor'ının içindedir. İkisi de şifreyi `/run/ankaros/session.sock` üzerinden
`ankaros-session`'a doğrulatır. `ankaros-session` soketteki sürecin kimliğine `SO_PEERCRED` ile bakar.
Şifre `crypt()` ile karşılaştırılır, PAM kullanılmaz. Otomatik giriş `/etc/ankaros/session.conf`'tadır.

## Sonuçlar

- Giriş ekranından masaüstüne geçerken compositor değişir, ekran kısa bir an kararır.
- PAM olmadığı için iki aşamalı doğrulama gibi PAM modülleri kullanılamaz. Gerekirse sonradan eklenir.
- Ekranı ilk açan süreç DRM yöneticisi olur. Bir oturum biterken compositor ekranı bırakmadan
  yenisi başlatılmamalıdır. Bu sırayı `ankaros-session` sağlar.
- Kapatma ve yeniden başlatma isteklerini yalnızca etkin oturumun kullanıcısı yapabilir.
  `ankaros-session` bu istekleri init'e iletir.
