// ankarosctl: ankaros-init'i denetler. Kullanım: ankarosctl help
#include "protocol.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <vector>

namespace {

const char *usage =
    "Kullanım: ankarosctl <komut>\n"
    "\n"
    "  status                  servislerin durumu\n"
    "  start|stop|restart AD   servisi başlat, durdur, yeniden başlat (root)\n"
    "  poweroff | reboot       sistemi kapat, yeniden başlat (root)\n"
    "  boot                    açılışta her adımın süresi\n"
    "  mark AD                 açılış süresine bir işaret ekle (ör. masaüstünün ilk karesi)\n";

std::vector<std::string> split(const std::string &line, char separator)
{
    std::vector<std::string> fields;
    size_t start = 0;
    for (;;) {
        const size_t end = line.find(separator, start);
        fields.push_back(line.substr(start, end - start));
        if (end == std::string::npos)
            return fields;
        start = end + 1;
    }
}

// Komutu gönderir, veri satırlarını lines'a koyar. Hata mesajını döner (başarılıysa boş).
std::string request(const std::string &command, std::vector<std::string> &lines)
{
    const int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path, ankaros::initSocketPath, sizeof address.sun_path - 1);
    if (fd < 0 || connect(fd, reinterpret_cast<sockaddr *>(&address), sizeof address) != 0)
        return std::string("ankaros-init'e bağlanılamadı: ") + strerror(errno);

    const std::string line = command + "\n";
    if (write(fd, line.data(), line.size()) != static_cast<ssize_t>(line.size()))
        return "komut gönderilemedi";
    std::string response;
    char buffer[4096];
    ssize_t n;
    while ((n = read(fd, buffer, sizeof buffer)) > 0)
        response.append(buffer, static_cast<size_t>(n));
    close(fd);

    lines = split(response, '\n');
    while (!lines.empty() && lines.back().empty())
        lines.pop_back();
    if (lines.empty())
        return "cevap yok";
    const std::string last = lines.back();
    lines.pop_back();
    if (last == "ok")
        return {};
    return last.starts_with("hata ") ? last.substr(5) : "beklenmeyen cevap: " + last;
}

// printf'in genişliği bayt sayar, Türkçe harfler iki bayt: genişlik karakter sayısına göre
// doldurulur
std::string pad(const std::string &text, size_t width)
{
    size_t characters = 0;
    for (unsigned char c : text)
        characters += (c & 0xC0) != 0x80;
    return text + std::string(width > characters ? width - characters : 0, ' ');
}

void printStatus(const std::vector<std::string> &lines)
{
    printf("%s %s %7s  %s\n", pad("SERVİS", 22).c_str(), pad("DURUM", 24).c_str(), "PID",
           "AÇIKLAMA");
    for (const std::string &line : lines) {
        const auto f = split(line, '\t');
        if (f.size() < 5)
            continue;
        std::string state = f[1];
        if (f[3] != "0")
            state += " (" + f[3] + ")";
        printf("%s %s %7s  %s\n", pad(f[0], 22).c_str(), pad(state, 24).c_str(), f[2].c_str(),
               f[4].c_str());
    }
}

void printBoot(const std::vector<std::string> &lines)
{
    double kernel = -1;
    double last = 0;
    printf("%s %9s %9s %9s\n", pad("ADIM", 24).c_str(), "BAŞLADI", "HAZIR", "SÜRE");
    for (const std::string &line : lines) {
        const auto f = split(line, '\t');
        if (f[0] == "service" && f.size() >= 5) {
            const double started = atof(f[2].c_str());
            const double ready = atof(f[3].c_str());
            if (started < 0) {
                printf("%s %9s %9s %9s  %s\n", pad(f[1], 24).c_str(), "-", "-", "-", f[4].c_str());
                continue;
            }
            if (kernel < 0 || started < kernel)
                kernel = started;
            if (ready >= 0) {
                printf("%s %9.3f %9.3f %9.3f\n", pad(f[1], 24).c_str(), started, ready,
                       ready - started);
                last = std::max(last, ready);
            } else {
                printf("%s %9.3f %9s %9s  %s\n", pad(f[1], 24).c_str(), started, "-", "-",
                       f[4].c_str());
            }
        } else if (f[0] == "mark" && f.size() >= 3) {
            const double at = atof(f[2].c_str());
            printf("%s %9s %9.3f\n", pad("◆ " + f[1], 24).c_str(), "", at);
            last = std::max(last, at);
        }
    }
    if (kernel >= 0)
        printf("\nİlk servis %.3f sn'de başladı (çekirdek ve init hazırlığı), son adım %.3f sn'de "
               "bitti.\n",
               kernel, last);
}

} // namespace

int main(int argc, char *argv[])
{
    if (argc < 2 || strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0) {
        fputs(usage, argc < 2 ? stderr : stdout);
        return argc < 2 ? 2 : 0;
    }
    std::string command = argv[1];
    for (int i = 2; i < argc; ++i)
        command += std::string(" ") + argv[i];

    std::vector<std::string> lines;
    const std::string error = request(command, lines);
    if (!error.empty()) {
        fprintf(stderr, "ankarosctl: %s\n", error.c_str());
        return 1;
    }
    if (command == "status")
        printStatus(lines);
    else if (command == "boot")
        printBoot(lines);
    else
        for (const std::string &line : lines)
            puts(line.c_str());
    return 0;
}
