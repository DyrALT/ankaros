#include "system.h"

#include "config.h"
#include "log.h"

#include <cerrno>
#include <csignal>
#include <cstring>
#include <ctime>
#include <string>
#include <sys/mount.h>
#include <sys/reboot.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

namespace ankaros {

namespace {

void mountFs(const char *source, const char *target, const char *type, unsigned long flags,
             const char *options = nullptr)
{
    mkdir(target, 0755);
    if (mount(source, target, type, flags, options) != 0 && errno != EBUSY)
        log::write("bağlanamadı: %s (%s): %s", target, type, strerror(errno));
}

void sleepFor(double seconds)
{
    timespec time{static_cast<time_t>(seconds),
                  static_cast<long>((seconds - static_cast<time_t>(seconds)) * 1e9)};
    nanosleep(&time, nullptr);
}

// Bütün çocuklar bitene ya da süre dolana kadar bekler
bool waitForChildren(double timeout)
{
    const double deadline = log::now() + timeout;
    while (reapChildren([](pid_t, int) {})) {
        if (log::now() >= deadline)
            return false;
        sleepFor(0.05);
    }
    return true;
}

} // namespace

const char *toString(PowerAction action)
{
    switch (action) {
    case PowerAction::Poweroff:
        return "kapatma";
    case PowerAction::Reboot:
        return "yeniden başlatma";
    case PowerAction::Halt:
        return "durdurma";
    }
    return "";
}

void prepareSystem()
{
    umask(022);
    setenv("PATH", "/usr/sbin:/usr/bin:/sbin:/bin", 1);

    mountFs("proc", "/proc", "proc", MS_NOSUID | MS_NODEV | MS_NOEXEC);
    mountFs("sysfs", "/sys", "sysfs", MS_NOSUID | MS_NODEV | MS_NOEXEC);
    mountFs("devtmpfs", "/dev", "devtmpfs", MS_NOSUID, "mode=0755");
    mountFs("devpts", "/dev/pts", "devpts", MS_NOSUID | MS_NOEXEC, "gid=5,mode=620,ptmxmode=0666");
    mountFs("tmpfs", "/dev/shm", "tmpfs", MS_NOSUID | MS_NODEV, "mode=1777");
    mountFs("tmpfs", "/run", "tmpfs", MS_NOSUID | MS_NODEV, "mode=0755");
    mountFs("tmpfs", "/tmp", "tmpfs", MS_NOSUID | MS_NODEV, "mode=1777");

    // Çekirdek kök dosya sistemini salt okunur bağlar
    if (mount(nullptr, "/", nullptr, MS_REMOUNT, nullptr) != 0)
        log::write("kök dosya sistemi yazılabilir yapılamadı: %s", strerror(errno));

    for (const auto &[target, link] :
         {std::pair{"/proc/self/fd", "/dev/fd"}, std::pair{"/proc/self/fd/0", "/dev/stdin"},
          std::pair{"/proc/self/fd/1", "/dev/stdout"},
          std::pair{"/proc/self/fd/2", "/dev/stderr"}}) {
        if (symlink(target, link) != 0 && errno != EEXIST)
            log::write("bağlantı oluşturulamadı: %s: %s", link, strerror(errno));
    }
    mkdir("/run/lock", 0755);
    mkdir("/run/ankaros", 0755);

    std::string hostname;
    if (readFile("/etc/hostname", hostname)) {
        hostname = std::string(trim(hostname));
        if (!hostname.empty() && sethostname(hostname.c_str(), hostname.size()) != 0)
            log::write("makine adı ayarlanamadı: %s", strerror(errno));
    }
}

void finishShutdown(PowerAction action)
{
    log::write("kalan süreçler sonlandırılıyor");
    kill(-1, SIGTERM);
    if (!waitForChildren(2.0)) {
        kill(-1, SIGKILL);
        waitForChildren(1.0);
    }

    sync();
    if (mount(nullptr, "/", nullptr, MS_REMOUNT | MS_RDONLY, nullptr) != 0)
        log::write("kök dosya sistemi salt okunur yapılamadı: %s", strerror(errno));
    sync();

    log::write("%s", toString(action));
    switch (action) {
    case PowerAction::Poweroff:
        reboot(RB_POWER_OFF);
        break;
    case PowerAction::Reboot:
        reboot(RB_AUTOBOOT);
        break;
    case PowerAction::Halt:
        reboot(RB_HALT_SYSTEM);
        break;
    }
    // reboot() yalnızca hata olursa döner. PID 1 çıkarsa çekirdek panik verir, bekle.
    log::write("reboot başarısız: %s", strerror(errno));
    for (;;)
        pause();
}

} // namespace ankaros
