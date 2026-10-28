#pragma once

#include <algorithm>
#include <functional>
#include <map>
#include <string>
#include <sys/types.h>
#include <vector>

namespace ankaros {

// Komut satırını alanlarına (sekmeyle ayrılmış) böler. Boş alanlar atlanır.
std::vector<std::string> splitFields(const std::string &line);

// Denetim soketi sunucusu (protokol: common/protocol.h). epoll ile ana döngüye bağlanır.
class ControlServer
{
public:
    struct Reply
    {
        std::vector<std::string> lines;
        std::string error; // boşsa "ok"
        // true: cevaptan sonra bağlantı açık kalır, broadcast() ile gelen her güncelleme bu
        // istemciye de gönderilir (ör. ağ durumunu izleyen alt çubuk)
        bool subscribe = false;
    };
    // Komut satırı ve bağlanan sürecin kullanıcı kimliği (SO_PEERCRED)
    using Handler = std::function<Reply(const std::vector<std::string> &command, uid_t uid)>;

    // Soket açılamazsa false döner, errno ayarlıdır
    bool listen(const char *path, int epollFd, Handler handler);
    // fd bu sunucuya aitse olayı işler ve true döner
    bool handle(int fd);
    // Abonelere veri satırlarını ve ardından "ok" satırını gönderir
    void broadcast(const std::vector<std::string> &lines);

private:
    void accept();
    void finish(int fd, const std::string &line);
    void drop(int fd);

    int m_listenFd = -1;
    int m_epollFd = -1;
    Handler m_handler;
    std::map<int, std::string> m_clients; // fd -> okunan veri
    std::vector<int> m_subscribers;
};

} // namespace ankaros
