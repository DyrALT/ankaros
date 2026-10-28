#pragma once

#include <string>

namespace ankaros {

// /etc/ankaros/session.conf. Biçim: docs/oturum.md
//   autologin = ankaros    # açılışta şifresiz oturum açılacak kullanıcı, boşsa giriş ekranı
struct SessionConfig
{
    std::string autologin;
};

SessionConfig parseSessionConfig(const std::string &text);
std::string formatSessionConfig(const SessionConfig &config);

} // namespace ankaros
