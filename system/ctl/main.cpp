// ankarosctl: ankaros-init'i denetler. Kullanım: ankarosctl help
#include "protocol.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
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
    const bool session = strcmp(argv[1], "session") == 0;
    const bool network = strcmp(argv[1], "network") == 0;
    std::vector<std::string> command(argv + (session || network ? 2 : 1), argv + argc);
    if (command.empty()) {
        fputs(usage, stderr);
        return 2;
    }

    std::vector<std::string> lines;
    const char *socketPath = session   ? ankaros::sessionSocketPath
                             : network ? ankaros::networkSocketPath
                                       : ankaros::initSocketPath;
    const std::string error = ankaros::request(socketPath, command, lines);
    if (!error.empty()) {
        fprintf(stderr, "ankarosctl: %s\n", error.c_str());
        return 1;
    }
    const bool init = !session && !network;
    if (init && command[0] == "status")
        printStatus(lines);
    else if (init && command[0] == "boot")
        printBoot(lines);
    else
        for (const std::string &line : lines)
            puts(line.c_str());
    return 0;
}
