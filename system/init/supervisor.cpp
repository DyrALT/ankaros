#include "supervisor.h"

#include "graph.h"
#include "log.h"

#include <algorithm>
#include <array>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <dirent.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <sys/wait.h>
#include <unistd.h>

namespace ankaros {

namespace {

constexpr const char *logDirectory = "/var/log";
constexpr double stopTimeout = 5.0;
// Art arda çöken servisin yeniden başlatılmadan önce bekleyeceği süreler (sn)
constexpr std::array restartDelays = {0.5, 1.0, 2.0, 4.0, 8.0, 16.0, 30.0};
// Bu kadar çalıştıktan sonra çöken servisin sayacı sıfırlanır
constexpr double stableRuntime = 10.0;

bool settled(ServiceState state)
{
    return state != ServiceState::Waiting && state != ServiceState::Starting;
}

std::string describeStatus(int status)
{
    if (WIFEXITED(status))
        return "çıkış kodu " + std::to_string(WEXITSTATUS(status));
    if (WIFSIGNALED(status))
        return std::string("sinyal ") + strsignal(WTERMSIG(status));
    return "durum " + std::to_string(status);
}

std::string joinCommand(const std::vector<std::string> &arguments)
{
    std::string text;
    for (const std::string &argument : arguments)
        text += (text.empty() ? "" : " ") + argument;
    return text;
}

void setEnvironment(std::vector<std::string> &environment, const std::string &entry)
{
    const std::string key = entry.substr(0, entry.find('=') + 1);
    std::erase_if(environment, [&](const std::string &e) { return e.starts_with(key); });
    environment.push_back(entry);
}

} // namespace

const char *toString(ServiceState state)
{
    static constexpr std::array names = {
        "bekliyor",  "başlatılıyor", "çalışıyor",           "tamamlandı",
        "başarısız", "durduruldu",   "yeniden başlatılacak"};
    return names[static_cast<size_t>(state)];
}

void Supervisor::load(const std::string &directory)
{
    DIR *dir = opendir(directory.c_str());
    if (!dir) {
        log::write("servis klasörü açılamadı: %s: %s", directory.c_str(), strerror(errno));
        return;
    }
    std::vector<ServiceDefinition> definitions;
    while (const dirent *entry = readdir(dir)) {
        const std::string file = entry->d_name;
        constexpr std::string_view suffix = ".service";
        if (file.size() <= suffix.size() || !file.ends_with(suffix))
            continue;
        const std::string name = file.substr(0, file.size() - suffix.size());
        std::string text;
        if (!readFile(directory + "/" + file, text)) {
            log::write("%s okunamadı", file.c_str());
            continue;
        }
        ServiceDefinition definition;
        std::vector<std::string> errors;
        if (!parseService(name, parseConfig(text), definition, errors)) {
            for (const std::string &error : errors)
                log::write("%s: %s", file.c_str(), error.c_str());
            log::write("%s atlandı", file.c_str());
            continue;
        }
        definitions.push_back(std::move(definition));
    }
    closedir(dir);

    for (const std::string &error : resolveDependencies(definitions))
        log::write("%s", error.c_str());
    for (const std::string &name : startOrder(definitions)) {
        auto it = std::find_if(definitions.begin(), definitions.end(),
                               [&](const ServiceDefinition &d) { return d.name == name; });
        add(std::move(*it));
    }
    log::write("%zu servis okundu", m_services.size());
}

void Supervisor::add(ServiceDefinition definition)
{
    Service service;
    service.definition = std::move(definition);
    m_services.push_back(std::move(service));
}

Service *Supervisor::find(const std::string &name)
{
    for (Service &service : m_services) {
        if (service.definition.name == name)
            return &service;
    }
    return nullptr;
}

bool Supervisor::dependenciesSettled(const Service &service) const
{
    for (const std::string &dependency : service.definition.after) {
        for (const Service &other : m_services) {
            if (other.definition.name == dependency && !settled(other.state))
                return false;
        }
    }
    return true;
}

void Supervisor::update()
{
    const double now = log::now();
    for (Service &service : m_services) {
        if (service.pid > 0 && service.killAt >= 0 && now >= service.killAt) {
            log::write("%s: durmadı, öldürülüyor", service.definition.name.c_str());
            kill(-service.pid, SIGKILL);
            service.killAt = -1;
        }
        if (service.state == ServiceState::RestartWait && now >= service.restartAt)
            spawn(service);
    }
    // Bir servisin başlaması başka bir servisi hazır yapabilir (simple), sırayla dolaşmak yeter:
    // m_services bağımlılık sırasında.
    for (Service &service : m_services) {
        if (service.state == ServiceState::Waiting && service.wanted &&
            dependenciesSettled(service))
            spawn(service);
    }
}

double Supervisor::timeout() const
{
    double earliest = -1;
    for (const Service &service : m_services) {
        for (double at : {service.state == ServiceState::RestartWait ? service.restartAt : -1.0,
                          service.pid > 0 ? service.killAt : -1.0}) {
            if (at >= 0 && (earliest < 0 || at < earliest))
                earliest = at;
        }
    }
    return earliest < 0 ? -1 : std::max(0.0, earliest - log::now());
}

void Supervisor::spawn(Service &service)
{
    const ServiceDefinition &definition = service.definition;
    const std::vector<std::string> &arguments = definition.commands.at(service.step);
    const double now = log::now();

    // Ortam: temel değişkenler, servisin kullanıcısı ve servis dosyasındaki environment satırları
    std::vector<std::string> environment = {"PATH=/usr/sbin:/usr/bin:/sbin:/bin", "HOME=/",
                                            "TERM=linux"};
    uid_t uid = 0;
    gid_t gid = 0;
    std::string home = "/";
    if (!definition.user.empty()) {
        const passwd *user = getpwnam(definition.user.c_str());
        if (!user) {
            log::write("%s: kullanıcı yok: %s", definition.name.c_str(), definition.user.c_str());
            finish(service, ServiceState::Failed);
            return;
        }
        uid = user->pw_uid;
        gid = user->pw_gid;
        home = user->pw_dir;
        setEnvironment(environment, "HOME=" + home);
        setEnvironment(environment, "USER=" + definition.user);
        setEnvironment(environment, "LOGNAME=" + definition.user);
        setEnvironment(environment, std::string("SHELL=") + user->pw_shell);
    }
    for (const std::string &entry : definition.environment)
        setEnvironment(environment, entry);

    std::vector<char *> argv;
    for (const std::string &argument : arguments)
        argv.push_back(const_cast<char *>(argument.c_str()));
    argv.push_back(nullptr);
    std::vector<char *> envp;
    for (const std::string &entry : environment)
        envp.push_back(const_cast<char *>(entry.c_str()));
    envp.push_back(nullptr);

    const std::string logPath = std::string(logDirectory) + "/" + definition.name + ".log";
    const int logFd = open(logPath.c_str(), O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
    if (logFd >= 0)
        dprintf(logFd, "[%8.3f] ankaros-init: %s\n", now, joinCommand(arguments).c_str());

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
        if (!definition.user.empty() && (initgroups(definition.user.c_str(), gid) != 0 ||
                                         setgid(gid) != 0 || setuid(uid) != 0)) {
            dprintf(STDERR_FILENO, "kullanıcıya geçilemedi: %s\n", strerror(errno));
            _exit(126);
        }
        if (chdir(home.c_str()) != 0 && chdir("/") != 0)
            _exit(126);
        execvpe(argv[0], argv.data(), envp.data());
        dprintf(STDERR_FILENO, "çalıştırılamadı: %s: %s\n", argv[0], strerror(errno));
        _exit(127);
    }
    if (logFd >= 0)
        close(logFd);

    if (pid < 0) {
        log::write("%s: fork başarısız: %s", definition.name.c_str(), strerror(errno));
        finish(service, ServiceState::Failed);
        return;
    }

    service.pid = pid;
    service.lastStartAt = now;
    if (service.startedAt < 0)
        service.startedAt = now;
    if (definition.type == ServiceType::Oneshot) {
        service.state = ServiceState::Starting;
    } else {
        service.state = ServiceState::Running;
        if (service.readyAt < 0)
            service.readyAt = now;
        log::write("%s: başlatıldı (pid %d)", definition.name.c_str(), pid);
    }
}

void Supervisor::finish(Service &service, ServiceState state)
{
    service.state = state;
    service.pid = 0;
    service.killAt = -1;
    service.step = 0;
}

bool Supervisor::childExited(pid_t pid, int status)
{
    auto it = std::find_if(m_services.begin(), m_services.end(),
                           [pid](const Service &s) { return s.pid == pid; });
    if (it == m_services.end())
        return false;
    Service &service = *it;
    const ServiceDefinition &definition = service.definition;
    const bool success = WIFEXITED(status) && WEXITSTATUS(status) == 0;
    const double now = log::now();
    service.pid = 0;
    service.killAt = -1;

    if (service.restartRequested) {
        service.restartRequested = false;
        service.wanted = true;
        service.restarts = 0;
        finish(service, ServiceState::Waiting);
        return true;
    }
    if (!service.wanted) {
        log::write("%s: durduruldu", definition.name.c_str());
        finish(service, ServiceState::Stopped);
        return true;
    }

    if (definition.type == ServiceType::Oneshot) {
        if (!success) {
            log::write("%s: başarısız (%s): %s", definition.name.c_str(),
                       describeStatus(status).c_str(),
                       joinCommand(definition.commands[service.step]).c_str());
            finish(service, ServiceState::Failed);
        } else if (++service.step < definition.commands.size()) {
            spawn(service);
        } else {
            if (service.readyAt < 0)
                service.readyAt = now;
            log::write("%s: tamamlandı (%.2f sn)", definition.name.c_str(),
                       now - service.lastStartAt);
            finish(service, ServiceState::Done);
        }
        return true;
    }

    const bool restart = definition.restart == RestartPolicy::Always ||
                         (definition.restart == RestartPolicy::OnFailure && !success);
    if (!restart) {
        log::write("%s: bitti (%s)", definition.name.c_str(), describeStatus(status).c_str());
        finish(service, success ? ServiceState::Stopped : ServiceState::Failed);
        return true;
    }
    if (now - service.lastStartAt >= stableRuntime)
        service.restarts = 0;
    const double delay =
        restartDelays[std::min(static_cast<size_t>(service.restarts), restartDelays.size() - 1)];
    ++service.restarts;
    service.state = ServiceState::RestartWait;
    service.restartAt = now + delay;
    log::write("%s: bitti (%s), %.1f sn sonra yeniden başlatılacak", definition.name.c_str(),
               describeStatus(status).c_str(), delay);
    return true;
}

void Supervisor::terminate(Service &service)
{
    if (service.pid <= 0)
        return;
    // Servis kendi oturumunun (setsid) lideri: sinyal bütün süreç grubuna gider. Etkileşimli
    // kabuklar SIGTERM'i yok sayar, SIGHUP ile kapanır (ör. getty'den açılmış oturum).
    for (int signal : {SIGTERM, SIGHUP, SIGCONT}) {
        if (kill(-service.pid, signal) != 0)
            kill(service.pid, signal);
    }
    service.killAt = log::now() + stopTimeout;
}

bool Supervisor::start(const std::string &name, std::string &error)
{
    Service *service = find(name);
    if (!service) {
        error = "böyle bir servis yok: " + name;
        return false;
    }
    if (service->pid > 0) {
        error = name + " zaten çalışıyor";
        return false;
    }
    service->wanted = true;
    service->restarts = 0;
    finish(*service, ServiceState::Waiting);
    return true;
}

bool Supervisor::stop(const std::string &name, std::string &error)
{
    Service *service = find(name);
    if (!service) {
        error = "böyle bir servis yok: " + name;
        return false;
    }
    service->wanted = false;
    service->restartRequested = false;
    if (service->pid > 0)
        terminate(*service);
    else if (service->state == ServiceState::Waiting || service->state == ServiceState::RestartWait)
        finish(*service, ServiceState::Stopped);
    return true;
}

bool Supervisor::restart(const std::string &name, std::string &error)
{
    Service *service = find(name);
    if (!service) {
        error = "böyle bir servis yok: " + name;
        return false;
    }
    if (service->pid > 0) {
        service->restartRequested = true;
        terminate(*service);
        return true;
    }
    return start(name, error);
}

void Supervisor::stopAll(double timeout)
{
    for (auto it = m_services.rbegin(); it != m_services.rend(); ++it) {
        it->wanted = false;
        it->restartRequested = false;
        terminate(*it);
    }
    const double deadline = log::now() + timeout;
    for (;;) {
        int status = 0;
        pid_t pid;
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
            childExited(pid, status);
        const bool running = std::any_of(m_services.begin(), m_services.end(),
                                         [](const Service &s) { return s.pid > 0; });
        if (!running || pid < 0 || log::now() >= deadline)
            break;
        usleep(50 * 1000);
    }
    for (const Service &service : m_services) {
        if (service.pid > 0)
            log::write("%s: zamanında durmadı", service.definition.name.c_str());
    }
}

void Supervisor::mark(const std::string &name)
{
    for (const auto &entry : m_marks) {
        if (entry.first == name)
            return; // yalnızca ilk zaman tutulur
    }
    m_marks.emplace_back(name, log::now());
    log::write("işaret: %s", name.c_str());
}

std::vector<std::string> Supervisor::statusLines() const
{
    std::vector<std::string> lines;
    for (const Service &service : m_services) {
        lines.push_back(service.definition.name + "\t" + toString(service.state) + "\t" +
                        (service.pid > 0 ? std::to_string(service.pid) : "-") + "\t" +
                        std::to_string(service.restarts) + "\t" + service.definition.description);
    }
    return lines;
}

std::vector<std::string> Supervisor::bootLines() const
{
    char buffer[128];
    std::vector<std::string> lines;
    for (const Service &service : m_services) {
        snprintf(buffer, sizeof buffer, "\t%.3f\t%.3f\t", service.startedAt, service.readyAt);
        lines.push_back("service\t" + service.definition.name + buffer + toString(service.state));
    }
    for (const auto &[name, time] : m_marks) {
        snprintf(buffer, sizeof buffer, "\t%.3f", time);
        lines.push_back("mark\t" + name + buffer);
    }
    return lines;
}

} // namespace ankaros
