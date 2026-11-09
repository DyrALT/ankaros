#include "appstore.h"

#include <algorithm>
#include <cstdint>
#include <cstdio>

namespace ankaros {

namespace {

bool endsWith(std::string_view text, std::string_view suffix)
{
    if (text.size() < suffix.size())
        return false;
    return std::equal(suffix.rbegin(), suffix.rend(), text.rbegin(), [](char lower, char c) {
        return lower == (c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c);
    });
}

std::vector<std::string_view> splitOn(std::string_view text, char separator)
{
    std::vector<std::string_view> parts;
    for (size_t start = 0;;) {
        const size_t end = text.find(separator, start);
        parts.push_back(text.substr(start, end == std::string_view::npos ? end : end - start));
        if (end == std::string_view::npos)
            return parts;
        start = end + 1;
    }
}

// Kayıt dosyası satırlara ve sekmelere bölündüğü için alanlarda bunlar bulunamaz
std::string singleLine(std::string_view text)
{
    std::string line(text);
    std::replace_if(
        line.begin(), line.end(), [](char c) { return c == '\t' || c == '\n' || c == '\r'; }, ' ');
    return line;
}

std::string jsonString(std::string_view text)
{
    std::string out = "\"";
    for (const char c : text) {
        if (c == '"' || c == '\\') {
            out += '\\';
            out += c;
        } else if (static_cast<unsigned char>(c) < 0x20) {
            char escaped[8];
            snprintf(escaped, sizeof escaped, "\\u%04x", c);
            out += escaped;
        } else {
            out += c;
        }
    }
    return out + "\"";
}

} // namespace

PackageKind packageKind(std::string_view fileName)
{
    if (endsWith(fileName, ".deb"))
        return PackageKind::Deb;
    if (endsWith(fileName, ".rpm"))
        return PackageKind::Rpm;
    if (endsWith(fileName, ".appimage"))
        return PackageKind::AppImage;
    if (endsWith(fileName, ".exe"))
        return PackageKind::Exe;
    return PackageKind::Unknown;
}

bool validPackageName(std::string_view name)
{
    if (name.size() < 2 || name.size() > 128 || name.front() == '-' || name.front() == '.' ||
        name.front() == '+')
        return false;
    return std::all_of(name.begin(), name.end(), [](char c) {
        return (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '+' || c == '-' || c == '.';
    });
}

DesktopEntry parseDesktopEntry(std::string_view text)
{
    DesktopEntry entry;
    // NUL içeren dosya tümden reddedilir (hiçbir şey okunmaz): atlanan bir bölüm başlığı sonraki
    // satırları ana girişe yazdırabilir, değer c_str() ile kesilince de anlam değişir
    if (text.find('\0') != std::string_view::npos)
        return entry;
    std::string localizedName;
    bool inEntry = false;
    for (std::string_view line : splitOn(text, '\n')) {
        if (!line.empty() && line.back() == '\r')
            line.remove_suffix(1);
        if (line.empty() || line.front() == '#')
            continue;
        if (line.front() == '[') {
            inEntry = line == "[Desktop Entry]";
            continue;
        }
        const size_t equals = line.find('=');
        if (!inEntry || equals == std::string_view::npos)
            continue;
        const std::string_view key = line.substr(0, equals);
        const std::string_view value = line.substr(equals + 1);
        if (key == "Name")
            entry.name = value;
        else if (key == "Name[tr]" || key == "Name[tr_TR]")
            localizedName = value;
        else if (key == "Exec")
            entry.exec = value;
        else if (key == "Icon")
            entry.icon = value;
        else if (key == "StartupWMClass")
            entry.wmClass = value;
        else if (key == "Type")
            entry.application = value == "Application";
        else if (key == "Terminal")
            entry.terminal = value == "true";
        else if ((key == "NoDisplay" || key == "Hidden") && value == "true")
            entry.hidden = true;
    }
    if (!localizedName.empty())
        entry.name = localizedName;
    return entry;
}

std::string cleanExec(std::string_view exec)
{
    std::string out;
    for (const std::string_view word : splitOn(exec, ' ')) {
        // Yalnızca yer tutucudan oluşan sözcük tamamen kalkar, boşluk bırakmaz
        if (word.empty() || (word.size() == 2 && word[0] == '%' && word[1] != '%'))
            continue;
        if (!out.empty())
            out += ' ';
        for (size_t i = 0; i < word.size(); ++i) {
            if (word[i] != '%' || i + 1 == word.size()) {
                out += word[i];
            } else if (word[++i] == '%') {
                out += '%';
            }
        }
    }
    return out;
}

InstalledApp makeInstalledApp(const std::string &package, const std::string &layer,
                              const std::string &desktopId, const DesktopEntry &entry)
{
    InstalledApp app;
    app.package = package;
    app.layer = layer;
    const std::string exec = cleanExec(entry.exec);
    if (!entry.application || entry.terminal || entry.hidden || entry.name.empty() || exec.empty())
        return app;
    app.name = singleLine(entry.name);
    app.command = singleLine("ankaros-katman " + layer + " -- " + exec);
    // Pencerenin Wayland app_id'si genellikle .desktop dosyasının adıdır ya da StartupWMClass
    app.appIds.push_back(singleLine(desktopId));
    if (!entry.wmClass.empty() && entry.wmClass != desktopId)
        app.appIds.push_back(singleLine(entry.wmClass));
    return app;
}

bool safeIconFile(std::string_view name)
{
    return !name.empty() && name.size() <= 255 && name != "." && name != ".." &&
           std::all_of(name.begin(), name.end(), [](char c) {
               return static_cast<unsigned char>(c) >= 0x20 && c != 0x7f && c != '/';
           });
}

uint64_t fnv1a(std::string_view text, uint64_t hash)
{
    for (const char c : text)
        hash = (hash ^ static_cast<unsigned char>(c)) * 1099511628211ull;
    return hash;
}

std::string iconFileName(std::string_view package, std::string_view id, std::string_view extension)
{
    std::string name = std::string(package) + '_' + std::string(id);
    for (char &c : name) {
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
              c == '.' || c == '-' || c == '_' || c == '+'))
            c = '_';
    }
    if (name.size() > 80)
        name.resize(80);
    // Temizlik farklı çiftleri aynı ada düşürebilir ("a b" ve "a_b"), özet bunu ayırır (FNV-1a)
    uint64_t hash = fnv1a(package);
    hash = fnv1a(std::string_view("\xff", 1), hash);
    hash = fnv1a(id, hash);
    char digest[20];
    snprintf(digest, sizeof digest, "%012llx", static_cast<unsigned long long>(hash >> 16));
    return name + '-' + digest + '.' + std::string(extension);
}

std::string formatInstalled(const std::vector<InstalledApp> &apps)
{
    std::string text;
    for (const InstalledApp &app : apps) {
        std::string ids;
        for (const std::string &id : app.appIds)
            ids += (ids.empty() ? "" : " ") + id;
        text += app.package + '\t' + app.layer + '\t' + app.name + '\t' + app.command + '\t' + ids +
                '\t' + singleLine(app.icon) + '\n';
    }
    return text;
}

std::vector<InstalledApp> parseInstalled(std::string_view text)
{
    std::vector<InstalledApp> apps;
    for (const std::string_view line : splitOn(text, '\n')) {
        const auto fields = splitOn(line, '\t');
        if (fields.size() != 5 && fields.size() != 6)
            continue;
        InstalledApp app;
        app.package = fields[0];
        app.layer = fields[1];
        app.name = fields[2];
        app.command = fields[3];
        for (const std::string_view id : splitOn(fields[4], ' ')) {
            if (!id.empty())
                app.appIds.emplace_back(id);
        }
        if (fields.size() == 6)
            app.icon = fields[5];
        apps.push_back(std::move(app));
    }
    return apps;
}

std::string formatCatalog(const std::vector<InstalledApp> &apps)
{
    std::string text = "{\n    \"apps\": [";
    bool first = true;
    for (const InstalledApp &app : apps) {
        if (app.name.empty())
            continue;
        text += first ? "\n" : ",\n";
        first = false;
        text += "        { \"name\": " + jsonString(app.name) +
                ", \"command\": " + jsonString(app.command) + ", \"appIds\": [";
        for (size_t i = 0; i < app.appIds.size(); ++i)
            text += (i ? ", " : "") + jsonString(app.appIds[i]);
        text += "]";
        if (!app.icon.empty())
            text += ", \"icon\": " + jsonString(app.icon);
        text += " }";
    }
    return text + (first ? "]\n}\n" : "\n    ]\n}\n");
}

} // namespace ankaros
