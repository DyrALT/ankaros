#pragma once

#include <string>
#include <vector>

namespace ankaros {

// /etc/ankaros/session.conf. Biçim: docs/oturum.md
//   autologin = ankaros    # açılışta şifresiz oturum açılacak kullanıcı, boşsa giriş ekranı
//   scale = auto           # arayüz ölçeği: auto (ekrana göre, displayScale), 1 ya da 2
struct SessionConfig
{
    std::string autologin;
    // 0: ekrana göre, 1 ya da 2: elle seçilmiş
    int scale = 0;
};

SessionConfig parseSessionConfig(const std::string &text);

// Kullanıcının görünüm ayarlarından (~/.config/ankaros/shell.conf, [appearance] bölümü, Qt'nin INI
// biçimi) giriş ekranına iletilecek ortam değişkenleri: ANKAROS_DARK_MODE ve kullanıcı duvar
// kağıdını kilitte de istiyorsa ANKAROS_WALLPAPER. Giriş ekranı başka bir kullanıcıyla çalıştığı
// için dosyayı kendisi okuyamaz (karar 0015).
std::vector<std::string> greeterAppearance(const std::string &shellConf);

// Kullanıcının klavye düzeni (shell.conf, [appearance] keyboardLayout: "tr", "tr-f" ya da "us")
// için compositor'ın ortam değişkenleri: XKB_DEFAULT_LAYOUT ve gerekirse XKB_DEFAULT_VARIANT. Seçim
// yoksa ya da bilinmiyorsa boş döner, /etc/profile.d'deki varsayılan (Türkçe Q) kullanılır. Giriş
// ekranı için ayrıca ANKAROS_KEYBOARD_LAYOUT (giriş ekranı kullanıcının dosyasını okuyamaz).
std::vector<std::string> keyboardEnvironment(const std::string &shellConf, bool greeter);
std::string formatSessionConfig(const SessionConfig &config);

// Arayüz ölçeği. session.conf'ta elle seçildiyse (configured 1 ya da 2) o, değilse ekranın tercih
// edilen çözünürlüğüne (DRM, ör. "2880x1800") göre: 2560 piksel ve daha geniş ekranlarda 2 (Retina,
// UTM'de "Retina modu"), diğerlerinde 1. Ölçek compositor'a QT_SCALE_FACTOR ile verilir,
// uygulamalar Wayland çıktısının ölçeğinden öğrenir.
int displayScale(const std::string &mode, int configured = 0);

} // namespace ankaros
