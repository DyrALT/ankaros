#include "control.h"

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

namespace ankaros {

std::vector<std::string> splitFields(const std::string &line)
{
    std::vector<std::string> fields;
    for (size_t start = 0; start <= line.size();) {
        const size_t end = std::min(line.find('\t', start), line.size());
        if (end > start)
            fields.push_back(line.substr(start, end - start));
        start = end + 1;
    }
    return fields;
}

namespace {

constexpr size_t maxRequest = 1024;

void sendAll(int fd, const std::string &data)
{
    size_t sent = 0;
    while (sent < data.size()) {
        const ssize_t n = send(fd, data.data() + sent, data.size() - sent, MSG_NOSIGNAL);
        if (n <= 0)
            return;
        sent += static_cast<size_t>(n);
    }
}

} // namespace

bool ControlServer::listen(const char *path, int epollFd, Handler handler)
{
    m_epollFd = epollFd;
    m_handler = std::move(handler);
    m_listenFd = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, path, sizeof address.sun_path - 1);
    unlink(path);
    if (m_listenFd < 0 ||
        bind(m_listenFd, reinterpret_cast<sockaddr *>(&address), sizeof address) ||
        ::listen(m_listenFd, 16) != 0)
        return false;
    // Herkes bağlanabilir. Yetki komut başına, bağlanan sürecin kullanıcısına göre denetlenir.
    chmod(path, 0666);
    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = m_listenFd;
    epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_listenFd, &event);
    return true;
}

void ControlServer::accept()
{
    for (;;) {
        const int fd = accept4(m_listenFd, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (fd < 0)
            return;
        epoll_event event{};
        event.events = EPOLLIN;
        event.data.fd = fd;
        epoll_ctl(m_epollFd, EPOLL_CTL_ADD, fd, &event);
        m_clients[fd];
    }
}

bool ControlServer::handle(int fd)
{
    if (fd == m_listenFd) {
        accept();
        return true;
    }
    if (std::find(m_subscribers.begin(), m_subscribers.end(), fd) != m_subscribers.end()) {
        // Abone bir şey göndermez, okunabilir olması bağlantıyı kapattığı anlamına gelir
        char buffer[64];
        if (read(fd, buffer, sizeof buffer) <= 0)
            drop(fd);
        return true;
    }
    auto it = m_clients.find(fd);
    if (it == m_clients.end())
        return false;

    char buffer[256];
    const ssize_t n = read(fd, buffer, sizeof buffer);
    if (n < 0 && (errno == EAGAIN || errno == EINTR))
        return true;
    if (n > 0)
        it->second.append(buffer, static_cast<size_t>(n));
    const size_t newline = it->second.find('\n');
    if (newline != std::string::npos)
        finish(fd, it->second.substr(0, newline));
    else if (n <= 0 || it->second.size() > maxRequest)
        finish(fd, it->second); // satır sonu olmadan kapatıldı ya da çok uzun
    return true;
}

void ControlServer::finish(int fd, const std::string &line)
{
    const std::vector<std::string> command = splitFields(line);

    ucred credentials{};
    socklen_t length = sizeof credentials;
    getsockopt(fd, SOL_SOCKET, SO_PEERCRED, &credentials, &length);

    std::string response;
    bool subscribe = false;
    if (command.empty()) {
        response = "hata boş komut\n";
    } else {
        const Reply reply = m_handler(command, credentials.uid);
        subscribe = reply.subscribe;
        for (const std::string &replyLine : reply.lines)
            response += replyLine + "\n";
        response += reply.error.empty() ? "ok\n" : "hata " + reply.error + "\n";
    }
    sendAll(fd, response);
    m_clients.erase(fd);
    if (!command.empty() && response.ends_with("ok\n") && subscribe) {
        m_subscribers.push_back(fd);
        return;
    }
    epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
}

void ControlServer::drop(int fd)
{
    std::erase(m_subscribers, fd);
    epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
}

void ControlServer::broadcast(const std::vector<std::string> &lines)
{
    std::string message;
    for (const std::string &line : lines)
        message += line + "\n";
    message += "ok\n";
    for (int fd : std::vector<int>(m_subscribers)) {
        if (send(fd, message.data(), message.size(), MSG_NOSIGNAL | MSG_DONTWAIT) !=
            static_cast<ssize_t>(message.size()))
            drop(fd);
    }
}

} // namespace ankaros
