// ankaros-network: kablolu ağ servisi. Ayrıntılar: docs/ag.md, karar 0011.
//
// /etc/ankaros/network.conf'a göre eth0'ı DHCP (BusyBox udhcpc) ya da statik IP ile açar. Durumu
// (kablo, adres, ağ geçidi, DNS) çekirdekten okur, netlink ile değişiklikleri fark eder ve
// /run/ankaros/network.sock'a abone olan istemcilere (alt çubuk, Ayarlar › Ağ) gönderir.
#include "config.h"
#include "control.h"
#include "netconfig.h"
#include "protocol.h"

#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <grp.h>
#include <ifaddrs.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <pwd.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace ankaros;

namespace {

constexpr const char *configPath = "/etc/ankaros/network.conf";
constexpr const char *interfaceName = "eth0";
constexpr double refreshDelay = 0.3; // ardışık netlink olaylarını tek güncellemede topla
constexpr double periodicRefresh =
    5.0; // resolv.conf gibi netlink'in bildirmediği değişiklikler için

double now()
{
    timespec time{};
    clock_gettime(CLOCK_MONOTONIC, &time);
    return static_cast<double>(time.tv_sec) + static_cast<double>(time.tv_nsec) / 1e9;
}

void say(const std::string &message)
{
    printf("%s\n", message.c_str());
    fflush(stdout);
}

pid_t spawn(const std::vector<std::string> &arguments)
{
    std::vector<char *> argv;
    for (const std::string &argument : arguments)
        argv.push_back(const_cast<char *>(argument.c_str()));
    argv.push_back(nullptr);
    const pid_t pid = fork();
    if (pid == 0) {
        sigset_t none;
        sigemptyset(&none);
        sigprocmask(SIG_SETMASK, &none, nullptr);
        execvp(argv[0], argv.data());
        _exit(127);
    }
    return pid;
}

// Kısa komutu (ip) çalıştırıp bitmesini bekler
bool run(const std::vector<std::string> &arguments)
{
    const pid_t pid = spawn(arguments);
    int status = 0;
    if (pid < 0 || waitpid(pid, &status, 0) != pid)
        return false;
    return WIFEXITED(status) && WEXITSTATUS(status) == 0;
}

std::string readTrimmed(const std::string &path)
{
    std::string text;
    readFile(path, text);
    return std::string(trim(text));
}

bool writeText(const char *path, const std::string &text)
{
    FILE *file = fopen(path, "w");
    if (!file)
        return false;
    const bool ok = fputs(text.c_str(), file) >= 0;
    return fclose(file) == 0 && ok;
}

// Kullanıcı netdev grubunda mı (ağ ayarını değiştirme yetkisi)
bool inNetdevGroup(uid_t uid)
{
    if (uid == 0)
        return true;
    const passwd *user = getpwuid(uid);
    const group *netdev = getgrnam("netdev");
    if (!user || !netdev)
        return false;
    gid_t groups[64];
    int count = 64;
    if (getgrouplist(user->pw_name, user->pw_gid, groups, &count) < 0)
        return false;
    for (int i = 0; i < count; ++i) {
        if (groups[i] == netdev->gr_gid)
            return true;
    }
    return false;
}

class NetworkService
{
public:
    void start();
    std::vector<std::string> status() const;
    std::string apply(const NetworkConfig &config);
    void childExited(pid_t pid);
    void update(ControlServer &control);
    double timeout() const;
    void requestRefresh() { m_refreshAt = std::min(m_refreshAt, now() + refreshDelay); }
    void shutdown();

private:
    void startDhcp();
    void stopDhcp();

    NetworkConfig m_config;
    pid_t m_dhcp = 0;
    double m_dhcpRestartAt = -1;
    double m_refreshAt = 0;
    std::vector<std::string> m_lastStatus;
};

void NetworkService::start()
{
    std::string text;
    readFile(configPath, text);
    NetworkConfig config = parseNetworkConfig(text);
    const std::string error = validateNetworkConfig(config);
    if (!error.empty()) {
        say("network.conf geçersiz (" + error + "), DHCP kullanılıyor");
        config = {};
    }
    run({"ip", "link", "set", "lo", "up"});
    run({"ip", "link", "set", interfaceName, "up"});
    apply(config);
}

void NetworkService::startDhcp()
{
    // -f: ön planda kal, -S: günlüğü syslog'a yaz. Adresi, rotayı ve resolv.conf'u BusyBox'ın
    // varsayılan betiği ayarlar (/usr/share/udhcpc/default.script).
    m_dhcp = spawn({"udhcpc", "-f", "-S", "-i", interfaceName});
    m_dhcpRestartAt = -1;
    say("DHCP başladı (pid " + std::to_string(m_dhcp) + ")");
}

void NetworkService::stopDhcp()
{
    m_dhcpRestartAt = -1;
    if (m_dhcp <= 0)
        return;
    // SIGTERM alan udhcpc adresi bırakmaz, -R olmadan çalıştığı için adresi aşağıda temizleriz
    kill(m_dhcp, SIGTERM);
    int status = 0;
    waitpid(m_dhcp, &status, 0);
    m_dhcp = 0;
}

std::string NetworkService::apply(const NetworkConfig &config)
{
    const std::string error = validateNetworkConfig(config);
    if (!error.empty())
        return error;
    stopDhcp();
    run({"ip", "addr", "flush", "dev", interfaceName});
    m_config = config;
    if (config.mode == "dhcp") {
        startDhcp();
    } else {
        if (!run({"ip", "addr", "add", config.address, "dev", interfaceName}))
            return "Adres ayarlanamadı";
        if (!config.gateway.empty() && !run({"ip", "route", "replace", "default", "via",
                                             config.gateway, "dev", interfaceName}))
            return "Ağ geçidi ayarlanamadı";
        if (!writeText("/etc/resolv.conf", formatResolvConf(config.dns)))
            return "DNS ayarlanamadı";
        say("statik adres: " + config.address);
    }
    requestRefresh();
    return {};
}

void NetworkService::childExited(pid_t pid)
{
    if (pid == m_dhcp) {
        m_dhcp = 0;
        say("udhcpc bitti, 2 sn sonra yeniden başlatılacak");
        m_dhcpRestartAt = now() + 2.0;
    }
}

std::vector<std::string> NetworkService::status() const
{
    const std::string sys = std::string("/sys/class/net/") + interfaceName;
    std::string address;
    ifaddrs *list = nullptr;
    if (getifaddrs(&list) == 0) {
        for (ifaddrs *entry = list; entry; entry = entry->ifa_next) {
            if (!entry->ifa_addr || entry->ifa_addr->sa_family != AF_INET ||
                strcmp(entry->ifa_name, interfaceName) != 0)
                continue;
            char text[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in *>(entry->ifa_addr)->sin_addr, text,
                      sizeof text);
            const uint32_t mask =
                ntohl(reinterpret_cast<sockaddr_in *>(entry->ifa_netmask)->sin_addr.s_addr);
            address = std::string(text) + "/" + std::to_string(__builtin_popcount(mask));
            break;
        }
        freeifaddrs(list);
    }
    std::string route, resolv;
    readFile("/proc/net/route", route);
    readFile("/etc/resolv.conf", resolv);
    std::string dns;
    for (const std::string &server : nameservers(resolv))
        dns += (dns.empty() ? "" : " ") + server;
    std::string configDns;
    for (const std::string &server : m_config.dns)
        configDns += (configDns.empty() ? "" : " ") + server;

    return {
        std::string("interface\t") + interfaceName,
        "carrier\t" + readTrimmed(sys + "/carrier"),
        "mac\t" + readTrimmed(sys + "/address"),
        "mode\t" + m_config.mode,
        "address\t" + address,
        "gateway\t" + defaultGateway(route, interfaceName),
        "dns\t" + dns,
        "config.address\t" + m_config.address,
        "config.gateway\t" + m_config.gateway,
        "config.dns\t" + configDns,
    };
}

void NetworkService::update(ControlServer &control)
{
    const double t = now();
    if (m_dhcpRestartAt >= 0 && t >= m_dhcpRestartAt && m_config.mode == "dhcp")
        startDhcp();
    if (t >= m_refreshAt) {
        m_refreshAt = t + periodicRefresh;
        std::vector<std::string> current = status();
        if (current != m_lastStatus) {
            m_lastStatus = std::move(current);
            control.broadcast(m_lastStatus);
        }
    }
}

double NetworkService::timeout() const
{
    double earliest = m_refreshAt;
    if (m_dhcpRestartAt >= 0)
        earliest = std::min(earliest, m_dhcpRestartAt);
    return std::max(0.0, earliest - now());
}

void NetworkService::shutdown()
{
    stopDhcp();
}

int openNetlink()
{
    const int fd = socket(AF_NETLINK, SOCK_RAW | SOCK_NONBLOCK | SOCK_CLOEXEC, NETLINK_ROUTE);
    sockaddr_nl address{};
    address.nl_family = AF_NETLINK;
    address.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR | RTMGRP_IPV4_ROUTE;
    if (fd < 0 || bind(fd, reinterpret_cast<sockaddr *>(&address), sizeof address) != 0) {
        say(std::string("netlink açılamadı: ") + strerror(errno));
        return -1;
    }
    return fd;
}

} // namespace

int main()
{
    if (getuid() != 0) {
        fprintf(stderr, "ankaros-network root olarak çalışmalıdır\n");
        return 1;
    }
    sigset_t signals;
    sigemptyset(&signals);
    for (int signal : {SIGCHLD, SIGTERM, SIGINT, SIGHUP, SIGPIPE})
        sigaddset(&signals, signal);
    sigprocmask(SIG_BLOCK, &signals, nullptr);
    const int signalFd = signalfd(-1, &signals, SFD_NONBLOCK | SFD_CLOEXEC);
    const int netlinkFd = openNetlink();
    const int epollFd = epoll_create1(EPOLL_CLOEXEC);
    for (int fd : {signalFd, netlinkFd}) {
        if (fd < 0)
            continue;
        epoll_event event{};
        event.events = EPOLLIN;
        event.data.fd = fd;
        epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &event);
    }

    NetworkService service;
    ControlServer control;
    mkdir("/run/ankaros", 0755);
    const bool listening = control.listen(
        networkSocketPath, epollFd, [&service](const std::vector<std::string> &command, uid_t uid) {
            ControlServer::Reply reply;
            const std::string &name = command[0];
            if (name == "status" || name == "watch") {
                reply.lines = service.status();
                reply.subscribe = name == "watch";
            } else if (name == "set" && command.size() >= 2) {
                if (!inNetdevGroup(uid)) {
                    reply.error = "Ağ ayarını değiştirme yetkiniz yok";
                    return reply;
                }
                NetworkConfig config;
                config.mode = command[1];
                if (config.mode == "static") {
                    config.address = command.size() > 2 ? command[2] : "";
                    config.gateway = command.size() > 3 ? command[3] : "";
                    config.dns = splitWords(command.size() > 4 ? command[4] : "");
                }
                reply.error = service.apply(config);
                if (reply.error.empty() && !writeText(configPath, formatNetworkConfig(config)))
                    reply.error = "Ayar kaydedilemedi";
            } else {
                reply.error = "bilinmeyen komut: " + name;
            }
            return reply;
        });
    if (!listening) {
        fprintf(stderr, "soket açılamadı: %s: %s\n", networkSocketPath, strerror(errno));
        return 1;
    }
    service.start();

    for (;;) {
        const double timeout = service.timeout();
        epoll_event events[16];
        const int count = epoll_wait(epollFd, events, 16, static_cast<int>(timeout * 1000) + 1);
        for (int i = 0; i < count; ++i) {
            const int fd = events[i].data.fd;
            if (fd == netlinkFd) {
                char buffer[8192];
                while (recv(netlinkFd, buffer, sizeof buffer, 0) > 0) {
                }
                service.requestRefresh();
            } else if (fd == signalFd) {
                signalfd_siginfo info{};
                while (read(signalFd, &info, sizeof info) == sizeof info) {
                    if (info.ssi_signo == SIGCHLD) {
                        int status = 0;
                        pid_t pid;
                        while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
                            service.childExited(pid);
                    } else if (info.ssi_signo == SIGTERM || info.ssi_signo == SIGINT) {
                        service.shutdown();
                        return 0;
                    }
                }
            } else {
                control.handle(fd);
            }
        }
        service.update(control);
    }
}
