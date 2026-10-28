#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace ankaros {

// "ad = değer" satırlarından oluşan yapılandırma dosyaları: servis dosyaları, session.conf,
// network.conf. Boş satırlar atlanır. '#' satır başında ya da önünde boşluk varsa yorumdur.
// Aynı ad birden çok kez yazılabilir (ör. birden çok exec satırı).
struct ConfigEntry
{
    std::string key;
    std::string value;
    int line = 0;
};

struct ConfigError
{
    int line = 0;
    std::string message;
};

struct Config
{
    std::vector<ConfigEntry> entries;
    std::vector<ConfigError> errors;

    bool has(std::string_view key) const;
    // Adın son değeri, yoksa fallback
    std::string value(std::string_view key, std::string_view fallback = {}) const;
    // Adın bütün değerleri, dosyadaki sırayla
    std::vector<std::string> values(std::string_view key) const;
};

Config parseConfig(std::string_view text);

// Dosyanın tamamını okur. Hata olursa false döner.
bool readFile(const std::string &path, std::string &contents);

std::string_view trim(std::string_view text);

} // namespace ankaros
