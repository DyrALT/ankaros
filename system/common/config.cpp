#include "config.h"

#include <cctype>
#include <fstream>
#include <sstream>

namespace ankaros {

namespace {

bool isKeyChar(char c)
{
    return std::islower(static_cast<unsigned char>(c)) ||
           std::isdigit(static_cast<unsigned char>(c)) || c == '-' || c == '_';
}

std::string_view stripComment(std::string_view line)
{
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '#' && (i == 0 || std::isspace(static_cast<unsigned char>(line[i - 1]))))
            return line.substr(0, i);
    }
    return line;
}

} // namespace

std::string_view trim(std::string_view text)
{
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.front())))
        text.remove_prefix(1);
    while (!text.empty() && std::isspace(static_cast<unsigned char>(text.back())))
        text.remove_suffix(1);
    return text;
}

bool Config::has(std::string_view key) const
{
    for (const ConfigEntry &entry : entries) {
        if (entry.key == key)
            return true;
    }
    return false;
}

std::string Config::value(std::string_view key, std::string_view fallback) const
{
    for (auto it = entries.rbegin(); it != entries.rend(); ++it) {
        if (it->key == key)
            return it->value;
    }
    return std::string(fallback);
}

std::vector<std::string> Config::values(std::string_view key) const
{
    std::vector<std::string> result;
    for (const ConfigEntry &entry : entries) {
        if (entry.key == key)
            result.push_back(entry.value);
    }
    return result;
}

Config parseConfig(std::string_view text)
{
    Config config;
    int lineNumber = 0;
    while (!text.empty()) {
        const size_t end = text.find('\n');
        std::string_view line = text.substr(0, end);
        text.remove_prefix(end == std::string_view::npos ? text.size() : end + 1);
        ++lineNumber;

        line = trim(stripComment(line));
        if (line.empty())
            continue;

        const size_t equals = line.find('=');
        if (equals == std::string_view::npos) {
            config.errors.push_back({lineNumber, "'=' yok"});
            continue;
        }
        const std::string_view key = trim(line.substr(0, equals));
        if (key.empty()) {
            config.errors.push_back({lineNumber, "ad boş"});
            continue;
        }
        bool valid = true;
        for (char c : key)
            valid = valid && isKeyChar(c);
        if (!valid) {
            config.errors.push_back(
                {lineNumber, "geçersiz ad: " + std::string(key) + " (küçük harf, rakam, - ve _)"});
            continue;
        }
        config.entries.push_back(
            {std::string(key), std::string(trim(line.substr(equals + 1))), lineNumber});
    }
    return config;
}

bool readFile(const std::string &path, std::string &contents)
{
    std::ifstream file(path, std::ios::binary);
    if (!file)
        return false;
    std::ostringstream stream;
    stream << file.rdbuf();
    contents = stream.str();
    return !file.bad();
}

} // namespace ankaros
