// ank-paket: ank kur, kaldir ve liste komutlarını ankaros-apps servisine iletir. Şifreyi
// terminalden okur, ank kabuk betiği olduğu için Unix soketine kendisi bağlanamaz.
#include "protocol.h"

#include <climits>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <termios.h>
#include <unistd.h>
#include <vector>

using namespace ankaros;

namespace {

// Terminalde yazılanı göstermeden bir satır okur
std::string readPassword()
{
    const int fd = open("/dev/tty", O_RDWR | O_CLOEXEC);
    const int input = fd >= 0 ? fd : STDIN_FILENO;
    termios saved = {};
    const bool isTerminal = tcgetattr(input, &saved) == 0;
    if (isTerminal) {
        termios silent = saved;
        silent.c_lflag &= ~static_cast<tcflag_t>(ECHO);
        tcsetattr(input, TCSAFLUSH, &silent);
    }
    fputs("Şifre: ", stderr);
    fflush(stderr);
    std::string password;
    char c;
    while (read(input, &c, 1) == 1 && c != '\n')
        password += c;
    if (isTerminal) {
        tcsetattr(input, TCSAFLUSH, &saved);
        fputs("\n", stderr);
    }
    if (fd >= 0)
        close(fd);
    return password;
}

int usage()
{
    fputs("Kullanım: ank kur DOSYA.deb | ank kaldir PAKET | ank liste\n", stderr);
    return 2;
}

} // namespace

int main(int argc, char *argv[])
{
    if (argc < 2)
        return usage();
    const std::string name = argv[1];
    std::vector<std::string> command = {name};
    if (name == "kur" || name == "kaldir") {
        if (argc != 3)
            return usage();
        std::string target = argv[2];
        if (name == "kur") {
            char resolved[PATH_MAX];
            if (!realpath(argv[2], resolved)) {
                fprintf(stderr, "ank: dosya bulunamadı: %s\n", argv[2]);
                return 1;
            }
            target = resolved;
        }
        if (target.find_first_of("\t\n") != std::string::npos) {
            fputs("ank: dosya adında sekme ya da satır sonu olamaz\n", stderr);
            return 1;
        }
        command.push_back(target);
        if (getuid() != 0) {
            const std::string password = readPassword();
            if (password.find('\t') != std::string::npos) {
                fputs("ank: şifre yanlış\n", stderr);
                return 1;
            }
            command.push_back(password);
        }
        puts(name == "kur" ? "Kuruluyor..." : "Kaldırılıyor...");
        fflush(stdout);
    } else if (name != "liste" || argc != 2) {
        return usage();
    }

    std::vector<std::string> lines;
    const std::string error = request(appsSocketPath, command, lines);
    if (!error.empty()) {
        fprintf(stderr, "ank: %s\n", error.c_str());
        return 1;
    }
    for (const std::string &line : lines) {
        const size_t tab = line.find('\t');
        const std::string package = line.substr(0, tab);
        const std::string app = tab == std::string::npos ? "" : line.substr(tab + 1);
        if (name == "kur")
            printf("Kuruldu: %s\n", app.empty()
                                        ? (package + " (başlat panelinde uygulaması yok)").c_str()
                                        : app.c_str());
        else
            printf("%s  %s\n", package.c_str(), app.c_str());
    }
    if (name == "liste" && lines.empty())
        puts("ank ile kurulmuş paket yok.");
    if (name == "kaldir")
        printf("Kaldırıldı: %s\n", argv[2]);
    return 0;
}
