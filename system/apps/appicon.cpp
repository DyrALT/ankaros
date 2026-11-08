#include "appicon.h"

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstdint>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>
#include <vector>

namespace ankaros {

namespace {

// hicolor boyutları, büyükten küçüğe
constexpr int pngSizes[] = {512, 256, 192, 128, 96, 72, 64, 48, 36, 32, 24, 22, 16};

class Fd
{
public:
    explicit Fd(int fd)
        : m_fd(fd)
    {}
    ~Fd()
    {
        if (m_fd >= 0)
            close(m_fd);
    }
    Fd(const Fd &) = delete;
    Fd &operator=(const Fd &) = delete;
    Fd &operator=(Fd &&other) noexcept
    {
        if (this != &other) {
            if (m_fd >= 0)
                close(m_fd);
            m_fd = std::exchange(other.m_fd, -1);
        }
        return *this;
    }
    int get() const { return m_fd; }

private:
    int m_fd;
};

// NUL dahil kontrol karakteri içeren yol, açılırken c_str() ile kesilip başka bir yola dönüşebilir
bool hasControlChar(std::string_view text)
{
    return std::any_of(text.begin(), text.end(), [](char c) {
        return static_cast<unsigned char>(c) < 0x20 || static_cast<unsigned char>(c) == 0x7f;
    });
}

std::optional<std::string> readRegular(int fd, size_t maxSize)
{
    struct stat info;
    if (fstat(fd, &info) != 0 || !S_ISREG(info.st_mode) || info.st_size <= 0 ||
        static_cast<size_t>(info.st_size) > maxSize)
        return std::nullopt;
    std::string data(static_cast<size_t>(info.st_size), '\0');
    size_t done = 0;
    while (done < data.size()) {
        const ssize_t n = read(fd, data.data() + done, data.size() - done);
        if (n < 0 && errno == EINTR)
            continue;
        if (n <= 0)
            return std::nullopt;
        done += static_cast<size_t>(n);
    }
    return data;
}

// SVG'de dış kaynağa gidebilecek her şey reddedilir: DTD/varlık tanımı, "#" ile başlamayan
// href'ler (file:, http:, göreli yol), @import ve url(...) içindeki dış adresler
bool safeSvg(const std::string &data)
{
    if (data.find("<svg") == std::string::npos)
        return false;
    std::string text = data;
    for (char &c : text)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    if (text.find("<!doctype") != std::string::npos || text.find("<!entity") != std::string::npos ||
        text.find("@import") != std::string::npos)
        return false;
    const auto skip = [&](size_t i) {
        while (i < text.size() && (text[i] == ' ' || text[i] == '\t' || text[i] == '\n' ||
                                   text[i] == '\r' || text[i] == '"' || text[i] == '\''))
            ++i;
        return i;
    };
    for (size_t at = text.find("href"); at != std::string::npos; at = text.find("href", at + 1)) {
        size_t i = at + 4;
        while (i < text.size() &&
               (text[i] == ' ' || text[i] == '\t' || text[i] == '\n' || text[i] == '\r'))
            ++i;
        if (i >= text.size() || text[i] != '=')
            continue;
        i = skip(i + 1);
        if (i >= text.size() || text[i] != '#')
            return false;
    }
    for (size_t at = text.find("url("); at != std::string::npos; at = text.find("url(", at + 1)) {
        const size_t i = skip(at + 4);
        if (i >= text.size() || text[i] != '#')
            return false;
    }
    return true;
}

// IHDR'den okunan boyut makul sınırı aşmamalı
bool safePng(const std::string &data)
{
    if (!data.starts_with("\x89PNG\r\n\x1a\n") || data.size() < 24 ||
        data.compare(12, 4, "IHDR") != 0)
        return false;
    const auto be32 = [&](size_t at) {
        return (static_cast<uint32_t>(static_cast<unsigned char>(data[at])) << 24) |
               (static_cast<uint32_t>(static_cast<unsigned char>(data[at + 1])) << 16) |
               (static_cast<uint32_t>(static_cast<unsigned char>(data[at + 2])) << 8) |
               static_cast<uint32_t>(static_cast<unsigned char>(data[at + 3]));
    };
    const uint32_t width = be32(16);
    const uint32_t height = be32(20);
    return width >= 1 && height >= 1 && width <= maxIconDimension && height <= maxIconDimension;
}

bool looksLike(std::string_view extension, const std::string &data)
{
    return extension == "png" ? safePng(data) : safeSvg(data);
}

std::optional<IconFile> tryRead(const std::string &root, const std::string &path,
                                std::string_view extension)
{
    std::optional<std::string> data = readRootFile(root, path, maxIconSize);
    if (!data || !looksLike(extension, *data))
        return std::nullopt;
    return IconFile{std::string(extension), std::move(*data)};
}

bool endsWith(std::string_view text, std::string_view suffix)
{
    return text.size() >= suffix.size() && text.substr(text.size() - suffix.size()) == suffix;
}

} // namespace

std::optional<std::string> readRootFile(const std::string &root, std::string_view path,
                                        size_t maxSize)
{
    if (hasControlChar(path))
        return std::nullopt;
    std::vector<std::string> parts;
    for (size_t start = 0; start < path.size();) {
        size_t end = path.find('/', start);
        if (end == std::string_view::npos)
            end = path.size();
        if (end > start)
            parts.emplace_back(path.substr(start, end - start));
        start = end + 1;
    }
    if (parts.empty() || std::any_of(parts.begin(), parts.end(), [](const std::string &part) {
            return part == "." || part == "..";
        }))
        return std::nullopt;
    Fd dir(open(root.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    for (size_t i = 0; i + 1 < parts.size() && dir.get() >= 0; ++i) {
        dir = Fd(
            openat(dir.get(), parts[i].c_str(), O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC));
    }
    if (dir.get() < 0)
        return std::nullopt;
    // O_NONBLOCK: boru ya da aygıt dosyası açılırken takılmasın, zaten S_ISREG ile elenir
    const Fd file(
        openat(dir.get(), parts.back().c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK));
    if (file.get() < 0)
        return std::nullopt;
    return readRegular(file.get(), maxSize);
}

std::optional<IconFile> readIcon(const std::string &root, std::string_view icon)
{
    if (icon.empty() || hasControlChar(icon))
        return std::nullopt;

    if (icon.front() == '/') {
        for (const char *extension : {"svg", "png"}) {
            if (endsWith(icon, std::string(".") + extension))
                return tryRead(root, std::string(icon), extension);
        }
        return std::nullopt;
    }

    // Simge adı tek bir dosya adıdır, yol olamaz
    if (icon.size() > 200 || icon.front() == '.' || icon.find('/') != std::string_view::npos)
        return std::nullopt;
    const std::string name(icon);
    std::vector<std::pair<std::string, const char *>> candidates = {
        {"usr/share/icons/hicolor/scalable/apps/" + name + ".svg", "svg"}};
    for (const int size : pngSizes) {
        const std::string dir = std::to_string(size) + "x" + std::to_string(size);
        candidates.emplace_back("usr/share/icons/hicolor/" + dir + "/apps/" + name + ".png", "png");
    }
    candidates.emplace_back("usr/share/pixmaps/" + name + ".svg", "svg");
    candidates.emplace_back("usr/share/pixmaps/" + name + ".png", "png");
    for (const auto &[path, extension] : candidates) {
        if (auto found = tryRead(root, path, extension))
            return found;
    }
    return std::nullopt;
}

} // namespace ankaros
