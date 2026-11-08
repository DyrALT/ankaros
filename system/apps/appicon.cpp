#include "appicon.h"

#include <cerrno>
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

std::vector<std::string> splitPath(std::string_view path)
{
    std::vector<std::string> parts;
    for (size_t start = 0; start < path.size();) {
        size_t end = path.find('/', start);
        if (end == std::string_view::npos)
            end = path.size();
        if (end > start)
            parts.emplace_back(path.substr(start, end - start));
        start = end + 1;
    }
    return parts;
}

std::optional<std::string> readRegular(int fd)
{
    struct stat info;
    if (fstat(fd, &info) != 0 || !S_ISREG(info.st_mode) || info.st_size <= 0 ||
        static_cast<size_t>(info.st_size) > maxIconSize)
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

// Yolu katman kökünden başlayıp bileşen bileşen açar. Hiçbir bileşen sembolik bağlantı olamaz:
// katmandaki bir bağlantı ana sistemdeki bir dosyaya işaret edebilir. ".." reddedilir.
std::optional<std::string> readFileIn(int rootFd, std::string_view path)
{
    const std::vector<std::string> parts = splitPath(path);
    if (parts.empty())
        return std::nullopt;
    for (const std::string &part : parts) {
        if (part == "." || part == "..")
            return std::nullopt;
    }
    Fd dir(dup(rootFd));
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
    return readRegular(file.get());
}

bool looksLike(std::string_view extension, const std::string &data)
{
    if (extension == "png")
        return data.starts_with("\x89PNG\r\n\x1a\n");
    return data.find("<svg") != std::string::npos;
}

std::optional<IconFile> tryRead(int rootFd, const std::string &path, std::string_view extension)
{
    std::optional<std::string> data = readFileIn(rootFd, path);
    if (!data || !looksLike(extension, *data))
        return std::nullopt;
    return IconFile{std::string(extension), std::move(*data)};
}

bool endsWith(std::string_view text, std::string_view suffix)
{
    return text.size() >= suffix.size() && text.substr(text.size() - suffix.size()) == suffix;
}

} // namespace

std::optional<IconFile> readIcon(const std::string &root, std::string_view icon)
{
    if (icon.empty())
        return std::nullopt;
    const Fd rootFd(open(root.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC));
    if (rootFd.get() < 0)
        return std::nullopt;

    if (icon.front() == '/') {
        for (const char *extension : {"svg", "png"}) {
            if (endsWith(icon, std::string(".") + extension))
                return tryRead(rootFd.get(), std::string(icon), extension);
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
        if (auto found = tryRead(rootFd.get(), path, extension))
            return found;
    }
    return std::nullopt;
}

} // namespace ankaros
