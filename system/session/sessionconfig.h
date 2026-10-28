#pragma once

#include <string>
#include <vector>

namespace ankaros {

// /etc/ankaros/session.conf. Biçim: docs/oturum.md
//   autologin = ankaros    # açılışta şifresiz oturum açılacak kullanıcı, boşsa giriş ekranı
struct SessionConfig
{
    std::string autologin;
};

SessionConfig parseSessionConfig(const std::string &text);

// Kullanıcının görünüm ayarlarından (~/.config/ankaros/shell.conf, [appearance] bölümü, Qt'nin INI
// biçimi) giriş ekranına iletilecek ortam değişkenleri: ANKAROS_DARK_MODE ve kullanıcı duvar
// kağıdını kilitte de istiyorsa ANKAROS_WALLPAPER. Giriş ekranı başka bir kullanıcıyla çalıştığı
// için dosyayı kendisi okuyamaz (karar 0015).
std::vector<std::string> greeterAppearance(const std::string &shellConf);
std::string formatSessionConfig(const SessionConfig &config);

} // namespace ankaros
