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

private:
    void startGreeter();
    void startSession(const std::string &user);
    bool spawn(Mode mode, const std::string &user);
    void terminate();
    bool allowAttempt(std::string &error);
    void failedAttempt();
    std::string sessionUser() const { return m_child.mode == Mode::Session ? m_child.user : ""; }

    Child m_child;
    std::string m_pendingUser; // giriş başarılı, giriş ekranı kapanınca açılacak oturum
    bool m_logoutRequested = false;
    int m_quickCrashes = 0;
    double m_restartAt = -1; // çöken compositor'ın yeniden başlatılma zamanı
    Mode m_restartMode = Mode::None;
    std::string m_restartUser;
    double m_killAt = -1;
    double m_nextAttemptAt = 0;
    bool m_shuttingDown = false;
};

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
    if (const passwd *shown = owner.empty() ? nullptr : getpwnam(owner.c_str()))
        readFile(std::string(shown->pw_dir) + "/.config/ankaros/shell.conf", conf);
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
    // Ortam değişkenlerinin geri kalanı (eglfs/KMS, dil, varsayılan klavye) /etc/profile.d altından
    // gelir
    const char *argv[] = {"/bin/sh",
                          "-c",
                          ". /etc/profile; exec /usr/bin/ankaros-compositor \"$@\"",
                          "ankaros-compositor",
                          mode == Mode::Greeter ? "--login" : nullptr,
                          nullptr};

    const int logFd = open(compositorLog, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
    if (logFd >= 0)
        dprintf(logFd, "--- ankaros-session: %s (%s) ---\n",
                mode == Mode::Greeter ? "giriş ekranı" : "oturum", user.c_str());

    const pid_t pid = fork();
    if (pid == 0) {
        sigset_t none;
        sigemptyset(&none);
        sigprocmask(SIG_SETMASK, &none, nullptr);
        setsid();
        const int nullFd = open("/dev/null", O_RDWR);
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
    m_child = {};
    m_killAt = -1;
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
        } else if (allowAttempt(reply.error) && !checkPassword(sessionUser(), command[1])) {
            failedAttempt();
            reply.error = "Şifre yanlış";
        }
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
    manager.start();

    for (;;) {
        const double timeout = manager.timeout();
        epoll_event events[16];
        const int count = epoll_wait(epollFd, events, 16,
                                     timeout < 0 ? -1 : static_cast<int>(timeout * 1000) + 1);
        for (int i = 0; i < count; ++i) {
            if (events[i].data.fd != signalFd) {
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
