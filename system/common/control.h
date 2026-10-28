#pragma once

#include <functional>
#include <map>
#include <string>
#include <sys/types.h>
#include <vector>

namespace ankaros {

// Denetim soketi sunucusu (protokol: common/protocol.h). epoll ile ana döngüye bağlanır.
class ControlServer
{
public:
    struct Reply
    {
        std::vector<std::string> lines;
        std::string error; // boşsa "ok"
    };
    // Komut satırı ve bağlanan sürecin kullanıcı kimliği (SO_PEERCRED)
    using Handler = std::function<Reply(const std::vector<std::string> &command, uid_t uid)>;

    bool listen(const char *path, int epollFd, Handler handler);
    // fd bu sunucuya aitse olayı işler ve true döner
    bool handle(int fd);

private:
    void accept();
    void finish(int fd, const std::string &line);

    int m_listenFd = -1;
    int m_epollFd = -1;
    Handler m_handler;
    std::map<int, std::string> m_clients; // fd -> okunan veri
};

} // namespace ankaros
