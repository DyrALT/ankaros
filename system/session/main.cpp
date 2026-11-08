// ankaros-session: oturum yöneticisi. Ayrıntılar: docs/oturum.md, karar 0010.
//
// root olarak çalışır ve ekranda her an tek bir compositor tutar: ya giriş ekranı
// ("ankaros-compositor --login", ankaros-login kullanıcısı) ya da giriş yapmış kullanıcının
// masaüstü. Şifreleri yalnızca bu süreç görür. Compositor'lar /run/ankaros/session.sock üzerinden
// giriş, kilit açma, oturumu kapatma ve kapatma isteklerini gönderir.
#include "accounts.h"
#include "config.h"
#include "control.h"
#include "protocol.h"
#include "sessionconfig.h"
#include "sessionlog.h"

#include <cerrno>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <sys/epoll.h>
#include <sys/signalfd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

using namespace ankaros;

namespace {

constexpr const char *configPath = "/etc/ankaros/session.conf";
constexpr const char *greeterUser = "ankaros-login";
constexpr const char *compositorLog = "/var/log/ankaros-compositor.log";
constexpr double failedLoginDelay = 1.5; // hatalı şifreden sonra yeni deneme için beklenen süre
constexpr double stopTimeout = 5.0;
constexpr int maxQuickCrashes = 3; // art arda bu kadar çöken oturum yerine giriş ekranı açılır
// Masaüstü günlüğünün en büyük boyutu, sonra .1 olur (kök dosya sistemini doldurmasın)
constexpr size_t compositorLogSize = 4 * 1024 * 1024;

double now()
{
    timespec time{};
    clock_gettime(CLOCK_MONOTONIC, &time);
    return static_cast<double>(time.tv_sec) + static_cast<double>(time.tv_nsec) / 1e9;
}

void say(const char *format, ...) __attribute__((format(printf, 1, 2)));
void say(const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    vprintf(format, arguments);
    va_end(arguments);
    putchar('\n');
    fflush(stdout);
}

enum class Mode
{
    None,
    Greeter,
    Session,
};

struct Child
{
    Mode mode = Mode::None;
    pid_t pid = 0;
    std::string user;
    uid_t uid = 0;
    double startedAt = 0;
};

class SessionManager
{
public:
    void start();
    void childExited(pid_t pid, int status);
    void update();
    double timeout() const;
    void shutdown();
    ControlServer::Reply handle(const std::vector<std::string> &command, uid_t peer);
    void setEpoll(int fd) { m_epollFd = fd; }
    // fd masaüstünün çıktı borusuysa okur ve true döner
    bool handleOutput(int fd);

private:
    void startGreeter();
    void startSession(const std::string &user);
    bool spawn(Mode mode, const std::string &user);
    void terminate();
    int openOutput();
    void closeOutput();
    bool allowAttempt(std::string &error);
    void failedAttempt();
    std::string sessionUser() const { return m_child.mode == Mode::Session ? m_child.user : ""; }

    Child m_child;
    std::string m_pendingUser; // giriş başarılı, giriş ekranı kapanınca açılacak oturum
    bool m_logoutRequested = false;
    // Oturumun ekranı kilitli (kabuk "lock" ile bildirir, doğru şifreli "unlock" açar). Kilitliyken
    // compositor çökerse oturum şifresiz yeniden açılmaz, giriş ekranına dönülür.
    bool m_locked = false;
    int m_quickCrashes = 0;
    double m_restartAt = -1; // çöken compositor'ın yeniden başlatılma zamanı
    Mode m_restartMode = Mode::None;
    std::string m_restartUser;
    double m_killAt = -1;
    double m_nextAttemptAt = 0;
    bool m_shuttingDown = false;
    // Masaüstünün çıktısı: uygulamalar borunun yazma ucunu devralır, oturum yöneticisi okuyup
    // günlüğe yazar (SessionLog)
    SessionLog m_log{compositorLog, compositorLogSize, now};
    int m_epollFd = -1;
    int m_output = -1;
};

int SessionManager::openOutput()
{
    closeOutput();
    int fds[2];
    if (pipe2(fds, O_CLOEXEC) != 0) {
        say("çıktı borusu açılamadı: %s", strerror(errno));
        return -1;
    }
    fcntl(fds[0], F_SETFL, fcntl(fds[0], F_GETFL) | O_NONBLOCK);
    m_output = fds[0];
    if (m_epollFd >= 0) {
        epoll_event event{};
        event.events = EPOLLIN;
        event.data.fd = m_output;
        epoll_ctl(m_epollFd, EPOLL_CTL_ADD, m_output, &event);
    }
    return fds[1];
}

void SessionManager::closeOutput()
{
    if (m_output < 0)
        return;
    handleOutput(m_output); // kalanı oku
    if (m_output < 0)
        return;
    if (m_epollFd >= 0)
        epoll_ctl(m_epollFd, EPOLL_CTL_DEL, m_output, nullptr);
    close(m_output);
    m_output = -1;
    m_log.flush();
}

bool SessionManager::handleOutput(int fd)
{
    if (fd < 0 || fd != m_output)
        return false;
    // Seviye tetiklemeli: bir seferde en fazla 256 KiB, kalanı sonraki turda (denetim soketi
    // beklemesin)
    char buffer[16 * 1024];
    for (int i = 0; i < 16; ++i) {
        const ssize_t count = read(fd, buffer, sizeof buffer);
        if (count > 0) {
            m_log.append(buffer, static_cast<size_t>(count));
            continue;
        }
        if (count < 0 && (errno == EAGAIN || errno == EINTR))
            break;
        // Bütün yazanlar kapandı (ya da hata): boru kapatılır
        if (m_epollFd >= 0)
            epoll_ctl(m_epollFd, EPOLL_CTL_DEL, fd, nullptr);
        close(fd);
        m_output = -1;
        m_log.flush();
        break;
    }
    return true;
}

SessionConfig readConfig()
{
    std::string text;
    readFile(configPath, text);
    return parseSessionConfig(text);
}

// Giriş ekranında gösterilen kullanıcı: uid'si 1000 ve üstü olan ilk kullanıcı (compositor'daki
// Session::firstLoginUser ile aynı kural)
std::string firstLoginUser()
{
    setpwent();
    std::string name;
    while (const passwd *entry = getpwent()) {
        if (entry->pw_uid >= 1000 && entry->pw_uid < 60000) {
            name = entry->pw_name;
            break;
        }
    }
    endpwent();
    return name;
}

// /run/user/<uid>: Wayland soketi ve Qt'nin çalışma zamanı dosyaları için kullanıcıya özel klasör
std::string prepareRuntimeDirectory(uid_t uid, gid_t gid)
{
    mkdir("/run/user", 0755);
    const std::string path = "/run/user/" + std::to_string(uid);
    mkdir(path.c_str(), 0700);
    if (chown(path.c_str(), uid, gid) != 0 || chmod(path.c_str(), 0700) != 0)
        say("%s hazırlanamadı: %s", path.c_str(), strerror(errno));
    return path;
}

void SessionManager::start()
{
    const SessionConfig config = readConfig();
    if (!config.autologin.empty() && getpwnam(config.autologin.c_str())) {
        say("otomatik giriş: %s", config.autologin.c_str());
        startSession(config.autologin);
    } else {
        if (!config.autologin.empty())
            say("otomatik giriş kullanıcısı yok: %s", config.autologin.c_str());
        startGreeter();
    }
}

void SessionManager::startGreeter()
{
    if (!spawn(Mode::Greeter, greeterUser))
        say("giriş ekranı başlatılamadı");
}

void SessionManager::startSession(const std::string &user)
{
    if (!spawn(Mode::Session, user)) {
        say("%s için oturum başlatılamadı, giriş ekranı açılıyor", user.c_str());
        startGreeter();
    }
}

// Bağlı ekranın tercih edilen çözünürlüğü ("3680x2300"): DRM'in ilk modu. Bulunamazsa boş.
std::string preferredMode()
{
    DIR *drm = opendir("/sys/class/drm");
    if (!drm)
        return {};
    std::string mode;
    while (const dirent *entry = readdir(drm)) {
        const std::string base = std::string("/sys/class/drm/") + entry->d_name;
        std::string status;
        if (!readFile(base + "/status", status) || status.rfind("connected", 0) != 0)
            continue;
        std::string modes;
        if (readFile(base + "/modes", modes) && !modes.empty()) {
            mode = modes.substr(0, modes.find('\n'));
            break;
        }
    }
    closedir(drm);
    return mode;
}

// Kullanıcının kendi dosyasını root olarak okumak yerine onun yetkisiyle okur: bağlantılarla
// root'un dosyalarına ulaşılamaz. Yalnızca düz dosya ve en fazla maxSize bayt (büyük bir dosya
// oturum yöneticisinin belleğini doldurmasın).
bool readUserFile(const passwd &account, const std::string &path, std::string &contents,
                  size_t maxSize = 64 * 1024)
{
    if (setegid(account.pw_gid) != 0)
        return false;
    if (seteuid(account.pw_uid) != 0) {
        if (setegid(0) != 0)
            abort();
        return false;
    }
    const int fd = open(path.c_str(), O_RDONLY | O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC);
    const bool restored = seteuid(0) == 0 && setegid(0) == 0;
    if (!restored) {
        say("yetki geri alınamadı: %s", strerror(errno));
        abort();
    }
    if (fd < 0)
        return false;
    struct stat info;
    if (fstat(fd, &info) != 0 || !S_ISREG(info.st_mode) ||
        static_cast<size_t>(info.st_size) > maxSize) {
        close(fd);
        return false;
    }
    contents.clear();
    char buffer[4096];
    ssize_t count;
    while ((count = read(fd, buffer, sizeof buffer)) > 0 && contents.size() < maxSize)
        contents.append(buffer, static_cast<size_t>(count));
    close(fd);
    if (contents.size() > maxSize)
        contents.resize(maxSize);
    return count >= 0;
}

bool SessionManager::spawn(Mode mode, const std::string &user)
{
    const passwd *account = getpwnam(user.c_str());
    if (!account) {
        say("kullanıcı yok: %s", user.c_str());
        return false;
    }
    const uid_t uid = account->pw_uid;
    const gid_t gid = account->pw_gid;
    const std::string runtime = prepareRuntimeDirectory(uid, gid);
    // Giriş ekranı kullanıcısının ev klasörü yok, ayarlarını çalışma zamanı klasörüne yazar
    const std::string home = mode == Mode::Greeter ? runtime : account->pw_dir;

    std::vector<std::string> environment = {
        "PATH=/usr/bin:/bin",
        "HOME=" + home,
        "USER=" + user,
        "LOGNAME=" + user,
        std::string("SHELL=") + account->pw_shell,
        "XDG_RUNTIME_DIR=" + runtime,
    };
    // Retina ekranlarda arayüz 2 kat büyüklükte çizilir (sessionconfig.h, displayScale)
    const std::string screenMode = preferredMode();
    const int scale = displayScale(screenMode, readConfig().scale);
    if (scale == 2)
        environment.push_back("QT_SCALE_FACTOR=2");
    say("ekran: %s, ölçek %d", screenMode.empty() ? "?" : screenMode.c_str(), scale);
    // Giriş ekranında gösterilen kullanıcının (oturumda kullanıcının kendisinin) tema, duvar kağıdı
    // ve klavye düzeni tercihi (karar 0015)
    std::string conf;
    const std::string owner = mode == Mode::Greeter ? firstLoginUser() : user;
    if (const passwd *shown = owner.empty() ? nullptr : getpwnam(owner.c_str())) {
        const passwd copy = *shown;
        readUserFile(copy, std::string(copy.pw_dir) + "/.config/ankaros/shell.conf", conf);
    }
    if (mode == Mode::Greeter) {
        for (const std::string &entry : greeterAppearance(conf))
            environment.push_back(entry);
    }
    for (const std::string &entry : keyboardEnvironment(conf, mode == Mode::Greeter))
        environment.push_back(entry);
    std::vector<char *> envp;
    for (std::string &entry : environment)
        envp.push_back(entry.data());
    envp.push_back(nullptr);
    // masaustu-baslat /etc/profile'ı okur (eglfs/KMS, dil, varsayılan klavye /etc/profile.d
    // altından gelir), oturumda önce ses altyapısını (D-Bus, PipeWire, WirePlumber) sonra
    // compositor'ı başlatır
    const char *argv[] = {"/bin/sh", "/usr/libexec/ankaros/masaustu-baslat",
                          mode == Mode::Greeter ? "--login" : nullptr, nullptr};

    // Önceki oturumdan kalan boru kapatılır, kalan yazanlar (oturumla birlikte öldürülür) EPIPE
    // alır
    const int logFd = openOutput();
    m_log.note(std::string("--- ankaros-session: ") +
               (mode == Mode::Greeter ? "giriş ekranı" : "oturum") + " (" + user + ") ---");

    const pid_t pid = fork();
    if (pid == 0) {
        sigset_t none;
        sigemptyset(&none);
        sigprocmask(SIG_SETMASK, &none, nullptr);
        setsid();
        const int nullFd = open("/dev/null", O_RDWR | O_CLOEXEC);
        dup2(nullFd, STDIN_FILENO);
        dup2(logFd >= 0 ? logFd : nullFd, STDOUT_FILENO);
        dup2(logFd >= 0 ? logFd : nullFd, STDERR_FILENO);
        if (initgroups(user.c_str(), gid) != 0 || setgid(gid) != 0 || setuid(uid) != 0) {
            dprintf(STDERR_FILENO, "kullanıcıya geçilemedi: %s\n", strerror(errno));
            _exit(126);
        }
        if (chdir(home.c_str()) != 0 && chdir("/") != 0)
            _exit(126);
        execve(argv[0], const_cast<char *const *>(argv), envp.data());
        dprintf(STDERR_FILENO, "çalıştırılamadı: %s\n", strerror(errno));
        _exit(127);
    }
    if (logFd >= 0)
        close(logFd);
    if (pid < 0) {
        say("fork başarısız: %s", strerror(errno));
        return false;
    }
    m_child = {mode, pid, user, uid, now()};
    m_logoutRequested = false;
    say("%s başladı: %s (pid %d)", mode == Mode::Greeter ? "giriş ekranı" : "oturum", user.c_str(),
        pid);
    return true;
}

void SessionManager::terminate()
{
    if (m_child.pid <= 0)
        return;
    for (int signal : {SIGTERM, SIGHUP, SIGCONT})
        kill(-m_child.pid, signal);
    m_killAt = now() + stopTimeout;
}

void SessionManager::childExited(pid_t pid, int status)
{
    if (pid != m_child.pid)
        return;
    const Child child = m_child;
    const bool wasLocked = m_locked;
    m_child = {};
    m_killAt = -1;
    m_locked = false;
    const bool clean = WIFEXITED(status) && WEXITSTATUS(status) == 0;
    say("%s bitti: %s (%s %d)", child.mode == Mode::Greeter ? "giriş ekranı" : "oturum",
        child.user.c_str(), WIFEXITED(status) ? "çıkış kodu" : "sinyal",
        WIFEXITED(status) ? WEXITSTATUS(status) : WTERMSIG(status));
    if (m_shuttingDown)
        return;

    if (child.mode == Mode::Greeter) {
        if (!m_pendingUser.empty()) {
            const std::string user = m_pendingUser;
            m_pendingUser.clear();
            m_quickCrashes = 0;
            startSession(user);
        } else {
            // Giriş ekranı kendiliğinden kapanmamalı: çöktü, kısa bir süre sonra yeniden aç
            m_restartMode = Mode::Greeter;
            m_restartAt = now() + 1.0;
        }
        return;
    }

    if (m_logoutRequested || clean) {
        m_quickCrashes = 0;
        startGreeter();
        return;
    }
    if (wasLocked) {
        say("oturum kilitliyken çöktü, giriş ekranı açılıyor");
        m_quickCrashes = 0;
        startGreeter();
        return;
    }
    // Oturum çöktü: aynı kullanıcıyla yeniden aç, art arda çöküyorsa giriş ekranına dön
    if (now() - child.startedAt < 10.0)
        ++m_quickCrashes;
    else
        m_quickCrashes = 0;
    if (m_quickCrashes >= maxQuickCrashes) {
        say("oturum art arda çöktü, giriş ekranı açılıyor");
        m_quickCrashes = 0;
        startGreeter();
        return;
    }
    m_restartMode = Mode::Session;
    m_restartUser = child.user;
    m_restartAt = now() + 1.0;
}

void SessionManager::update()
{
    const double t = now();
    if (m_child.pid > 0 && m_killAt >= 0 && t >= m_killAt) {
        say("compositor durmadı, öldürülüyor");
        kill(-m_child.pid, SIGKILL);
        m_killAt = -1;
    }
    if (m_restartAt >= 0 && t >= m_restartAt && m_child.pid <= 0) {
        m_restartAt = -1;
        if (m_restartMode == Mode::Session)
            startSession(m_restartUser);
        else
            startGreeter();
    }
}

double SessionManager::timeout() const
{
    double earliest = -1;
    for (double at : {m_restartAt, m_child.pid > 0 ? m_killAt : -1.0}) {
        if (at >= 0 && (earliest < 0 || at < earliest))
            earliest = at;
    }
    return earliest < 0 ? -1 : std::max(0.0, earliest - now());
}

void SessionManager::shutdown()
{
    m_shuttingDown = true;
    terminate();
    const double deadline = now() + stopTimeout;
    while (m_child.pid > 0 && now() < deadline) {
        int status = 0;
        const pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid > 0)
            childExited(pid, status);
        else
            usleep(50 * 1000);
    }
    if (m_child.pid > 0)
        kill(-m_child.pid, SIGKILL);
}

bool SessionManager::allowAttempt(std::string &error)
{
    if (now() < m_nextAttemptAt) {
        error = "Biraz bekleyip tekrar deneyin";
        return false;
    }
    return true;
}

void SessionManager::failedAttempt()
{
    m_nextAttemptAt = now() + failedLoginDelay;
}

ControlServer::Reply SessionManager::handle(const std::vector<std::string> &command, uid_t peer)
{
    ControlServer::Reply reply;
    const std::string &name = command[0];
    const size_t args = command.size() - 1;
    const bool fromChild = peer == 0 || (m_child.pid > 0 && peer == m_child.uid);
    const bool fromGreeter = m_child.mode == Mode::Greeter && fromChild;
    const bool fromSession = m_child.mode == Mode::Session && fromChild;

    if (name == "status" && args == 0) {
        reply.lines.push_back(std::string("mode\t") + (m_child.mode == Mode::Greeter   ? "greeter"
                                                       : m_child.mode == Mode::Session ? "session"
                                                                                       : "none"));
        reply.lines.push_back("user\t" + m_child.user);
        reply.lines.push_back("autologin\t" + readConfig().autologin);
    } else if (name == "login" && args == 2) {
        if (!fromGreeter) {
            reply.error = "giriş yalnızca giriş ekranından yapılabilir";
        } else if (allowAttempt(reply.error)) {
            const std::string &user = command[1];
            const passwd *account = getpwnam(user.c_str());
            // Giriş ekranı kullanıcısı ve sistem kullanıcıları (uid < 1000) oturum açamaz
            if (!account || account->pw_uid < 1000 || !checkPassword(user, command[2])) {
                failedAttempt();
                reply.error = "Şifre yanlış";
                say("hatalı giriş denemesi: %s", user.c_str());
            } else {
                m_pendingUser = user;
                terminate();
            }
        }
    } else if (name == "unlock" && args == 1) {
        if (!fromSession || peer == 0) {
            reply.error = "kilit yalnızca oturumdaki kullanıcı tarafından açılabilir";
        } else if (allowAttempt(reply.error)) {
            if (checkPassword(sessionUser(), command[1])) {
                m_locked = false;
            } else {
                failedAttempt();
                reply.error = "Şifre yanlış";
            }
        }
    } else if (name == "lock" && args == 0) {
        if (!fromSession || peer == 0)
            reply.error = "kilit yalnızca oturumdaki kullanıcı tarafından bildirilebilir";
        else
            m_locked = true;
    } else if (name == "logout" && args == 0) {
        if (!fromSession) {
            reply.error = "açık bir oturum yok";
        } else {
            m_logoutRequested = true;
            terminate();
        }
    } else if ((name == "poweroff" || name == "reboot") && args == 0) {
        if (!fromChild) {
            reply.error = "bu komut için yetkiniz yok";
        } else {
            std::vector<std::string> ignored;
            reply.error = request(initSocketPath, {name}, ignored);
        }
    } else if (name == "autologin" && args == 1 && (command[1] == "on" || command[1] == "off")) {
        if (!fromSession || peer == 0) {
            reply.error =
                "otomatik giriş yalnızca oturumdaki kullanıcı tarafından değiştirilebilir";
        } else {
            // Diğer ayarlar (ölçek) korunur
            SessionConfig config = readConfig();
            config.autologin = command[1] == "on" ? sessionUser() : "";
            const std::string text = formatSessionConfig(config);
            FILE *file = fopen(configPath, "w");
            if (!file || fputs(text.c_str(), file) < 0 || fclose(file) != 0)
                reply.error = std::string("ayar kaydedilemedi: ") + strerror(errno);
        }
    } else if (name == "passwd" && args == 2) {
        if (!fromSession || peer == 0) {
            reply.error = "şifre yalnızca oturumdaki kullanıcı tarafından değiştirilebilir";
        } else if (allowAttempt(reply.error)) {
            if (!checkPassword(sessionUser(), command[1])) {
                failedAttempt();
                reply.error = "Mevcut şifre yanlış";
            } else if (command[2].size() < 4) {
                reply.error = "Yeni şifre en az 4 karakter olmalı";
            } else {
                setPassword(sessionUser(), command[2], reply.error);
                if (reply.error.empty())
                    say("şifre değiştirildi: %s", sessionUser().c_str());
            }
        }
    } else if (name == "status" || name == "login" || name == "unlock" || name == "lock" ||
               name == "logout" || name == "poweroff" || name == "reboot" || name == "autologin" ||
               name == "passwd") {
        // Alan sayısı ya da değeri yanlış. Boş alanlar atlandığı için boş şifre de buraya düşer.
        reply.error = "yanlış argüman sayısı ya da boş alan: " + name;
    } else {
        reply.error = "bilinmeyen komut: " + name;
    }
    return reply;
}

} // namespace

int main()
{
    if (getuid() != 0) {
        fprintf(stderr, "ankaros-session root olarak çalışmalıdır\n");
        return 1;
    }
    sigset_t signals;
    sigemptyset(&signals);
    for (int signal : {SIGCHLD, SIGTERM, SIGINT, SIGHUP, SIGPIPE})
        sigaddset(&signals, signal);
    sigprocmask(SIG_BLOCK, &signals, nullptr);
    const int signalFd = signalfd(-1, &signals, SFD_NONBLOCK | SFD_CLOEXEC);
    const int epollFd = epoll_create1(EPOLL_CLOEXEC);
    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = signalFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, signalFd, &event);

    SessionManager manager;
    ControlServer control;
    mkdir("/run/ankaros", 0755);
    if (!control.listen(sessionSocketPath, epollFd,
                        [&manager](const std::vector<std::string> &command, uid_t uid) {
                            return manager.handle(command, uid);
                        })) {
        fprintf(stderr, "soket açılamadı: %s: %s\n", sessionSocketPath, strerror(errno));
        return 1;
    }
    manager.setEpoll(epollFd);
    manager.start();

    for (;;) {
        const double timeout = manager.timeout();
        epoll_event events[16];
        const int count = epoll_wait(epollFd, events, 16,
                                     timeout < 0 ? -1 : static_cast<int>(timeout * 1000) + 1);
        for (int i = 0; i < count; ++i) {
            if (events[i].data.fd != signalFd) {
                if (!manager.handleOutput(events[i].data.fd))
                    control.handle(events[i].data.fd);
                continue;
            }
            signalfd_siginfo info{};
            while (read(signalFd, &info, sizeof info) == sizeof info) {
                if (info.ssi_signo == SIGCHLD) {
                    int status = 0;
                    pid_t pid;
                    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
                        manager.childExited(pid, status);
                } else if (info.ssi_signo == SIGTERM || info.ssi_signo == SIGINT) {
                    say("durduruluyor");
                    manager.shutdown();
                    return 0;
                }
            }
        }
        manager.update();
    }
}
