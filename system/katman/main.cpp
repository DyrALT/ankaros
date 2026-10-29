// ankaros-katman: bir komutu bir uyumluluk katmanının içinde çalıştırır (karar 0016).
//
//   ankaros-katman debian -- foot          kullanıcı olarak (root yetkisi olmadan)
//   ankaros-katman debian -- apt install x  root olarak (paket kurulumu, ank katman bunu kullanır)
//
// Kullanıcı olarak çalışınca yeni bir kullanıcı ad alanı ve bağlama ad alanı açılır: kullanıcı
// kendi kimliğiyle kalır, yalnızca bu ad alanında dosya sistemi görünümünü değiştirebilir. Katmanın
// kök dosya sistemine geçilir (chroot), ankaros'tan aygıtlar, ev klasörü ve Wayland soketi
// paylaşılır (layer.h). Katman bir sandbox değildir: uygulama kullanıcının dosyalarına erişir.
#include "layer.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <sched.h>
#include <string>
#include <sys/mount.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

namespace {

int fail(const char *what)
{
    std::fprintf(stderr, "ankaros-katman: %s: %s\n", what, std::strerror(errno));
    return 1;
}

bool writeFile(const char *path, const std::string &text)
{
    const int fd = open(path, O_WRONLY | O_CLOEXEC);
    if (fd < 0)
        return false;
    const bool ok = write(fd, text.data(), text.size()) == ssize_t(text.size());
    close(fd);
    return ok;
}

// Klasörü ve üstlerini oluşturur (mkdir -p)
void makeDirectories(const std::string &path)
{
    for (size_t slash = path.find('/', 1); slash != std::string::npos;
         slash = path.find('/', slash + 1))
        mkdir(path.substr(0, slash).c_str(), 0755);
    mkdir(path.c_str(), 0755);
}

bool isRegularFile(const std::string &path)
{
    struct stat info = {};
    return lstat(path.c_str(), &info) == 0 && S_ISREG(info.st_mode);
}

} // namespace

int main(int argc, char *argv[])
{
    int first = 2;
    if (argc >= 3 && std::strcmp(argv[2], "--") == 0)
        first = 3;
    if (argc <= first || !ankaros::validLayerName(argv[1])) {
        std::fprintf(stderr, "Kullanım: ankaros-katman <katman> -- <komut> [argüman...]\n");
        return 2;
    }
    const std::string root = ankaros::layerRoot(argv[1]);
    if (access((root + "/usr/bin").c_str(), F_OK) != 0) {
        std::fprintf(stderr, "ankaros-katman: \"%s\" katmanı kurulu değil (ank katman kur %s)\n",
                     argv[1], argv[1]);
        return 1;
    }

    const uid_t uid = getuid();
    const gid_t gid = getgid();
    if (uid == 0) {
        if (unshare(CLONE_NEWNS) != 0)
            return fail("bağlama ad alanı açılamadı");
    } else {
        if (unshare(CLONE_NEWUSER | CLONE_NEWNS) != 0)
            return fail("kullanıcı ad alanı açılamadı");
        // Kullanıcı ad alanında da aynı kimlik: dosyalar kullanıcınınsa öyle görünür
        if (!writeFile("/proc/self/setgroups", "deny") ||
            !writeFile("/proc/self/uid_map",
                       std::to_string(uid) + " " + std::to_string(uid) + " 1") ||
            !writeFile("/proc/self/gid_map",
                       std::to_string(gid) + " " + std::to_string(gid) + " 1"))
            return fail("kimlik eşlemesi yazılamadı");
    }
    // Bağlamalar ankaros'a sızmasın
    if (mount(nullptr, "/", nullptr, MS_REC | MS_PRIVATE, nullptr) != 0)
        return fail("bağlamalar ayrılamadı");

    // /run ve /home katmanda boş, yazılabilir: paylaşılan klasörler burada oluşturulur
    for (const char *dir : {"/run", "/home"}) {
        if (mount("tmpfs", (root + dir).c_str(), "tmpfs", MS_NOSUID | MS_NODEV, "mode=0755") != 0)
            return fail(dir);
    }

    const char *home = std::getenv("HOME");
    const char *runtime = std::getenv("XDG_RUNTIME_DIR");
    const auto paths =
        ankaros::sharedPaths(uid == 0 || !home ? "" : home, uid == 0 || !runtime ? "" : runtime);
    for (const auto &path : paths) {
        const std::string target = root + path.target;
        if (path.file) {
            // Katmandaki dosya düzgün bir dosya değilse (bağlantı) atla: ank katman kur bunu
            // hazırlar
            if (!isRegularFile(target))
                continue;
        } else {
            makeDirectories(target);
        }
        if (mount(path.source.c_str(), target.c_str(), nullptr, MS_BIND | MS_REC, nullptr) != 0)
            return fail(path.source.c_str());
    }

    if (chroot(root.c_str()) != 0)
        return fail("katmana geçilemedi");
    if (chdir(home && uid != 0 ? home : "/") != 0 && chdir("/") != 0)
        return fail("klasör");
    // Katmanın kendi programları önce
    setenv("PATH", "/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin", 1);
    execvp(argv[first], argv + first);
    return fail(argv[first]);
}
