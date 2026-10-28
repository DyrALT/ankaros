#include "sessionconfig.h"

#include "config.h"

namespace ankaros {

SessionConfig parseSessionConfig(const std::string &text)
{
    const Config config = parseConfig(text);
    return {config.value("autologin")};
}

std::string formatSessionConfig(const SessionConfig &config)
{
    return "# ankaros oturum ayarları (docs/oturum.md). Ayarlar › Kullanıcı'dan da değiştirilir.\n"
           "# Açılışta şifre sorulmadan oturum açılacak kullanıcı. Boşsa giriş ekranı açılır.\n"
           "autologin = " +
           config.autologin + "\n";
}

} // namespace ankaros
