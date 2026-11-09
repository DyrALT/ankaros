// ankaros-x11: X11 uygulamaları için Xwayland'i ihtiyaç olunca başlatır (karar 0019,
// docs/xwayland.md).
//
// Masaüstü açılınca masaustu-baslat bunu kullanıcı olarak çalıştırır. X ekranının soketini
// (/tmp/.X11-unix/X0) kendisi açar ve bekler. İlk X11 uygulaması bağlanınca xwayland-satellite'i
// soketle birlikte başlatır (-listenfd), satellite de Xwayland'i. Hiç X11 uygulaması açılmazsa ne
// Xwayland ne satellite çalışır. satellite çıkarsa (ya da çökerse) soket yine beklenir.
//
// Soket yalnızca dosya sisteminde açılır, soyut ad alanında (@/tmp/.X11-unix/X0) açılmaz: soyut
// soketin izni yoktur, makinedeki herkes bağlanıp ekranı ve klavyeyi okuyabilirdi. Dosya soketi
// yalnızca kullanıcıya açıktır (0600). Debian katmanı /tmp'yi paylaştığı için içindeki uygulamalar
// da bağlanabilir.
#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <fcntl.h>
#include <poll.h>
#include <string>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <unistd.h>

namespace {

constexpr int display = 0;
const std::string socketDir = "/tmp/.X11-unix";
const std::string socketPath = socketDir + "/X" + std::to_string(display);
const std::string lockPath = "/tmp/.X" + std::to_string(display) + "-lock";

volatile sig_atomic_t stopRequested = 0;

void say(const char *format, const char *detail = nullptr)
{
    std::fprintf(stderr, "ankaros-x11: ");
    std::fprintf(stderr, format, detail ? detail : std::strerror(errno));
    std::fputc('\n', stderr);
}

void onSignal(int)
{
    stopRequested = 1;
}

bool processAlive(pid_t pid)
{
    return pid > 0 && (kill(pid, 0) == 0 || errno == EPERM);
}

// X'in kilit dosyası: içinde sahibinin pid'i (11 karakter, X sunucularının biçimi). Önceki
// oturumdan kalan kilit (sahibi yaşamıyor) silinir.
bool takeLock()
{
    for (int attempt = 0; attempt < 2; ++attempt) {
        const int fd =
            open(lockPath.c_str(), O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0444);
        if (fd >= 0) {
            char text[16];
            const int length =
                std::snprintf(text, sizeof text, "%10d\n", static_cast<int>(getpid()));
            const bool written = write(fd, text, length) == length;
            close(fd);
            return written;
        }
        if (errno != EEXIST)
            break;
        const int existing = open(lockPath.c_str(), O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
        struct stat info;
        if (existing < 0 || fstat(existing, &info) != 0 || !S_ISREG(info.st_mode) ||
            info.st_uid != getuid()) {
            if (existing >= 0)
                close(existing);
            say("%s başka bir kullanıcının ya da bozuk, X11 uygulamaları açılamayacak",
                lockPath.c_str());
            return false;
        }
        char text[16] = {};
        const ssize_t length = read(existing, text, sizeof text - 1);
        close(existing);
        const pid_t owner = length > 0 ? static_cast<pid_t>(std::atoi(text)) : 0;
        if (processAlive(owner)) {
            say("X ekranı :0 kullanımda (%s), X11 uygulamaları açılamayacak", lockPath.c_str());
            return false;
        }
        unlink(lockPath.c_str());
    }
    say("kilit dosyası oluşturulamadı: %s");
    return false;
}

int openSocket()
{
    // Klasörü init açılışta root olarak 1777 oluşturur. Yoksa ya da başkasının bir klasörü değilse
    // (biri önceden kendi klasörünü koymuşsa) sokete güvenilmez.
    struct stat info;
    if (lstat(socketDir.c_str(), &info) != 0 || !S_ISDIR(info.st_mode) ||
        (info.st_uid != 0 && info.st_uid != getuid()) || (info.st_mode & 01000) == 0) {
        say("%s yok ya da güvenli değil", socketDir.c_str());
        return -1;
    }
    // Kilit bizde: kalan soket önceki oturumdan
    unlink(socketPath.c_str());

    const int fd = socket(AF_UNIX, SOCK_STREAM | SOCK_CLOEXEC, 0);
    if (fd < 0) {
        say("soket açılamadı: %s");
        return -1;
    }
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    std::strncpy(address.sun_path, socketPath.c_str(), sizeof address.sun_path - 1);
    const mode_t oldMask = umask(077);
    const bool bound = bind(fd, reinterpret_cast<sockaddr *>(&address), sizeof address) == 0;
    umask(oldMask);
    if (!bound || listen(fd, 16) != 0) {
        say("soket dinlenemedi: %s");
        close(fd);
        return -1;
    }
    return fd;
}

// satellite'i soketle başlatır. Soket çocukta kapanmasın diye CLOEXEC yalnızca orada kalkar.
pid_t startSatellite(int fd)
{
    const pid_t pid = fork();
    if (pid != 0)
        return pid;
    signal(SIGTERM, SIG_DFL);
    signal(SIGHUP, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    const int flags = fcntl(fd, F_GETFD);
    if (flags < 0 || fcntl(fd, F_SETFD, flags & ~FD_CLOEXEC) != 0)
        _exit(127);
    const std::string displayName = ":" + std::to_string(display);
    const std::string fdText = std::to_string(fd);
    execlp("xwayland-satellite", "xwayland-satellite", displayName.c_str(), "-listenfd",
           fdText.c_str(), static_cast<char *>(nullptr));
    _exit(127);
}

void cleanUp(int fd)
{
    if (fd >= 0)
        close(fd);
    unlink(socketPath.c_str());
    unlink(lockPath.c_str());
}

} // namespace

int main()
{
    struct sigaction action = {};
    action.sa_handler = onSignal;
    sigemptyset(&action.sa_mask);
    // SA_RESTART yok: sinyal poll'u ve waitpid'i bölsün
    for (int signalNumber : {SIGTERM, SIGHUP, SIGINT})
        sigaction(signalNumber, &action, nullptr);

    if (!std::getenv("WAYLAND_DISPLAY")) {
        say("%s", "WAYLAND_DISPLAY yok");
        return 1;
    }
    if (!takeLock())
        return 1;
    const int fd = openSocket();
    if (fd < 0) {
        unlink(lockPath.c_str());
        return 1;
    }

    int quickExits = 0;
    while (!stopRequested) {
        pollfd waiting{fd, POLLIN, 0};
        const int ready = poll(&waiting, 1, -1);
        if (ready < 0) {
            if (errno == EINTR)
                continue;
            say("poll: %s");
            break;
        }
        if ((waiting.revents & POLLIN) == 0) {
            say("%s", "soket kapandı");
            break;
        }

        const time_t started = time(nullptr);
        const pid_t child = startSatellite(fd);
        if (child < 0) {
            say("fork: %s");
            break;
        }
        int status = 0;
        while (waitpid(child, &status, 0) < 0 && errno == EINTR) {
            if (stopRequested)
                kill(child, SIGTERM);
        }
        if (stopRequested)
            break;
        if (WIFEXITED(status) && WEXITSTATUS(status) == 127) {
            say("%s", "xwayland-satellite çalıştırılamadı");
            break;
        }
        // Hemen çöküp duran satellite bağlanmaya çalışan her uygulamada yeniden başlamasın:
        // üst üste beş hızlı çıkıştan sonra vazgeçilir
        quickExits = time(nullptr) - started < 5 ? quickExits + 1 : 0;
        if (quickExits >= 5) {
            say("%s", "xwayland-satellite üst üste çöktü, X11 uygulamaları açılamayacak");
            break;
        }
        say("%s", "xwayland-satellite çıktı, X11 uygulamaları için yeniden beklenecek");
    }
    cleanUp(fd);
    return 0;
}
