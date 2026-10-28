#include "sessionconfig.h"

#include "config.h"

#include <algorithm>

namespace ankaros {

SessionConfig parseSessionConfig(const std::string &text)
{
    const Config config = parseConfig(text);
    return {config.value("autologin")};
}

std::vector<std::string> greeterAppearance(const std::string &shellConf)
{
    std::string section, darkMode, wallpaper;
    std::string onLock = "true";
    size_t start = 0;
    while (start < shellConf.size()) {
        const size_t end = std::min(shellConf.find('\n', start), shellConf.size());
        const std::string_view line = trim(std::string_view(shellConf).substr(start, end - start));
        start = end + 1;
        if (line.starts_with('[') && line.ends_with(']')) {
            section = std::string(line.substr(1, line.size() - 2));
            continue;
        }
        const size_t equals = line.find('=');
        if (section != "appearance" || equals == std::string_view::npos)
            continue;
        const std::string key(trim(line.substr(0, equals)));
        const std::string value(trim(line.substr(equals + 1)));
        if (key == "darkMode")
            darkMode = value;
        else if (key == "wallpaper")
            wallpaper = value;
        else if (key == "wallpaperOnLock")
            onLock = value;
    }
    std::vector<std::string> environment;
    if (darkMode == "true" || darkMode == "false")
        environment.push_back("ANKAROS_DARK_MODE=" + darkMode);
    // Yalnızca bilinen adlar iletilir: dosya kullanıcının, içeriği güvenilmez
    const bool known = wallpaper == "ankara-tasi" || wallpaper == "bozkir" ||
                       wallpaper == "kehribar" || wallpaper == "gece";
    if (onLock != "false" && known)
        environment.push_back("ANKAROS_WALLPAPER=" + wallpaper);
    return environment;
}

std::string formatSessionConfig(const SessionConfig &config)
{
    return "# ankaros oturum ayarları (docs/oturum.md). Ayarlar › Kullanıcı'dan da değiştirilir.\n"
           "# Açılışta şifre sorulmadan oturum açılacak kullanıcı. Boşsa giriş ekranı açılır.\n"
           "autologin = " +
           config.autologin + "\n";
}

} // namespace ankaros
