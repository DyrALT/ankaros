#include "protocol.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace ankaros {

std::string request(const char *socketPath, const std::vector<std::string> &command,
                    std::vector<std::string> &lines)
{
    lines.clear();
    const int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, socketPath, sizeof address.sun_path - 1);
    if (fd < 0 || connect(fd, reinterpret_cast<sockaddr *>(&address), sizeof address) != 0) {
        const std::string error =
            std::string("bağlanılamadı: ") + socketPath + ": " + strerror(errno);
        if (fd >= 0)
            close(fd);
        return error;
    }

    std::string line;
    for (const std::string &field : command)
        line += (line.empty() ? "" : "\t") + field;
    line += "\n";
    if (send(fd, line.data(), line.size(), MSG_NOSIGNAL) != static_cast<ssize_t>(line.size())) {
        close(fd);
        return "komut gönderilemedi";
    }
    std::string response;
    char buffer[4096];
    ssize_t n;
    while ((n = read(fd, buffer, sizeof buffer)) > 0)
        response.append(buffer, static_cast<size_t>(n));
    close(fd);

    for (size_t start = 0; start < response.size();) {
        const size_t end = std::min(response.find('\n', start), response.size());
        lines.push_back(response.substr(start, end - start));
        start = end + 1;
    }
    if (lines.empty())
        return "cevap yok";
    const std::string last = lines.back();
    lines.pop_back();
    if (last == "ok")
        return {};
    return last.starts_with("hata ") ? last.substr(5) : "beklenmeyen cevap: " + last;
}

} // namespace ankaros
