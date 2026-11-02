// ankaros-apps: paketleri uyumluluk katmanlarına kurar ve kaldırır, uygulama listesini günceller
// (docs/katmanlar.md, karar 0016). Root olarak çalışır. Paket betikleri katmanda root olarak
// çalıştığı için kurma ve kaldırma kullanıcının şifresini ister. İstekler sırayla işlenir, kurulum
// bitene kadar yeni istek cevap almaz.
#include "accounts.h"
#include "appstore.h"
#include "control.h"
#include "layer.h"
#include "protocol.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <filesystem>
#include <fstream>
#include <pwd.h>
#include <signal.h>
#include <sstream>
#include <string>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

using namespace ankaros;

namespace {

constexpr const char *registryPath = "/var/lib/ankaros/kurulu.tsv";
constexpr const char *catalogPath = "/var/lib/ankaros/uygulamalar.json";
constexpr const char *logPath = "/var/log/ankaros-apps.log";
constexpr const char *layerName = "debian";
const std::string logHint = std::string(" (ayrıntı: ") + logPath + ")";

std::string readFile(const std::string &path)
{
    std::ifstream file(path, std::ios::binary);
    std::stringstream text;
    text << file.rdbuf();
    return text.str();
}

// Geçici dosyaya yazıp yerine taşır: AppCatalog dosyayı izler, yarım dosya okumamalı
bool writeFile(const std::string &path, const std::string &text)
{
    const std::string temporary = path + ".yeni";
    const int fd = open(temporary.c_str(), O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
    if (fd < 0)
        return false;
    const bool ok = write(fd, text.data(), text.size()) == static_cast<ssize_t>(text.size());
    close(fd);
    return ok && rename(temporary.c_str(), path.c_str()) == 0;
}

// Komutu çalıştırır, çıktısı günlüğe (output verilmişse oraya) gider. Çıkış kodunu döner.
int run(const std::vector<std::string> &arguments, std::string *output = nullptr)
{
    int pipeFds[2] = {-1, -1};
    if (output && pipe2(pipeFds, O_CLOEXEC) != 0)
        return -1;
    const pid_t pid = fork();
    if (pid < 0)
        return -1;
    if (pid == 0) {
        const int null = open("/dev/null", O_RDONLY);
        dup2(null, STDIN_FILENO);
        const int log = open(logPath, O_WRONLY | O_CREAT | O_APPEND, 0640);
        dup2(output ? pipeFds[1] : log, STDOUT_FILENO);
        dup2(log, STDERR_FILENO);
        std::vector<char *> argv;
        for (const std::string &argument : arguments)
            argv.push_back(const_cast<char *>(argument.c_str()));
        argv.push_back(nullptr);
        execvp(argv[0], argv.data());
        _exit(127);
    }
    if (output) {
        close(pipeFds[1]);
        char buffer[4096];
        ssize_t n;
        while ((n = read(pipeFds[0], buffer, sizeof buffer)) > 0)
            output->append(buffer, static_cast<size_t>(n));
        close(pipeFds[0]);
    }
    int status = 0;
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
    }
    return WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

int runInLayer(std::vector<std::string> arguments, std::string *output = nullptr)
{
    arguments.insert(arguments.begin(), {"ankaros-katman", layerName, "--"});
    return run(arguments, output);
}

std::vector<InstalledApp> loadRegistry()
{
    return parseInstalled(readFile(registryPath));
}

bool saveRegistry(const std::vector<InstalledApp> &apps)
{
    return writeFile(registryPath, formatInstalled(apps)) &&
           writeFile(catalogPath, formatCatalog(apps));
}

// Paketin .desktop dosyalarından kayıtlar. Hiç uygulaması yoksa yine de bir kayıt döner, paket
// listede görünsün ve kaldırılabilsin.
std::vector<InstalledApp> readPackageApps(const std::string &package)
{
    std::vector<InstalledApp> apps;
    std::string files;
    runInLayer({"dpkg", "-L", package}, &files);
    std::stringstream lines(files);
    const std::string directory = "/usr/share/applications/";
    const std::string suffix = ".desktop";
    for (std::string line; std::getline(lines, line);) {
        if (!line.starts_with(directory) || !line.ends_with(suffix))
            continue;
        const std::string id =
            line.substr(directory.size(), line.size() - directory.size() - suffix.size());
        if (id.find('/') != std::string::npos)
            continue;
        const InstalledApp app = makeInstalledApp(
            package, layerName, id, parseDesktopEntry(readFile(layerRoot(layerName) + line)));
        if (!app.name.empty())
            apps.push_back(app);
    }
    if (apps.empty())
        apps.push_back({package, layerName, "", "", {}});
    return apps;
}

std::string unsupported(PackageKind kind)
{
    switch (kind) {
    case PackageKind::Rpm:
    case PackageKind::AppImage:
        return "Bu paket türü henüz desteklenmiyor, şimdilik yalnızca .deb kurulur";
    case PackageKind::Exe:
        return "Windows programları henüz desteklenmiyor";
    default:
        return "Bilinmeyen paket türü, şimdilik yalnızca .deb kurulur";
    }
}

// Paketi geçici bir klasöre kopyalar: katman ankaros'tan yalnızca /tmp'yi görür
std::string copyToTemporary(const std::string &path, std::string &directory)
{
    char pattern[] = "/tmp/ankaros-kur-XXXXXX";
    if (!mkdtemp(pattern))
        return "Geçici klasör oluşturulamadı";
    directory = pattern;
    // apt paketleri ayrı bir kullanıcıyla da okur
    chmod(directory.c_str(), 0755);
    const std::string target = directory + "/paket.deb";
    std::error_code error;
    std::filesystem::copy_file(path, target, error);
    if (error)
        return "Paket okunamadı: " + path;
    chmod(target.c_str(), 0644);
    return {};
}

ControlServer::Reply install(const std::string &path)
{
    ControlServer::Reply reply;
    struct stat info = {};
    if (stat(path.c_str(), &info) != 0 || !S_ISREG(info.st_mode)) {
        reply.error = "Dosya bulunamadı: " + path;
        return reply;
    }
    const PackageKind kind = packageKind(path);
    if (kind != PackageKind::Deb) {
        reply.error = unsupported(kind);
        return reply;
    }
    if (access((layerRoot(layerName) + "/usr/bin").c_str(), F_OK) != 0 &&
        run({"ank", "katman", "kur", layerName}) != 0) {
        reply.error = "Debian katmanı kurulamadı" + logHint;
        return reply;
    }

    std::string directory;
    reply.error = copyToTemporary(path, directory);
    if (reply.error.empty()) {
        const std::string target = directory + "/paket.deb";
        std::string package;
        runInLayer({"dpkg-deb", "-f", target, "Package"}, &package);
        while (!package.empty() && (package.back() == '\n' || package.back() == '\r'))
            package.pop_back();
        if (!validPackageName(package)) {
            reply.error = "Geçerli bir .deb paketi değil";
        } else if (runInLayer({"apt-get", "install", "-y", "-q", target}) != 0) {
            reply.error = "Paket kurulamadı" + logHint;
        } else {
            std::vector<InstalledApp> apps = loadRegistry();
            std::erase_if(apps, [&](const InstalledApp &app) { return app.package == package; });
            for (const InstalledApp &app : readPackageApps(package)) {
                apps.push_back(app);
                reply.lines.push_back(package + '\t' + app.name);
            }
            if (!saveRegistry(apps))
                reply.error = "Uygulama listesi yazılamadı";
        }
    }
    if (!directory.empty()) {
        std::error_code ignored;
        std::filesystem::remove_all(directory, ignored);
    }
    return reply;
}

ControlServer::Reply remove(const std::string &package)
{
    ControlServer::Reply reply;
    std::vector<InstalledApp> apps = loadRegistry();
    const size_t before = apps.size();
    std::erase_if(apps, [&](const InstalledApp &app) { return app.package == package; });
    if (!validPackageName(package) || apps.size() == before) {
        reply.error = "Bu paket ank ile kurulmamış: " + package;
    } else if (runInLayer({"apt-get", "purge", "--autoremove", "-y", "-q", package}) != 0) {
        reply.error = "Paket kaldırılamadı" + logHint;
    } else if (!saveRegistry(apps)) {
        reply.error = "Uygulama listesi yazılamadı";
    }
    return reply;
}

// Root şifre vermez. Diğer kullanıcılar kendi şifrelerini verir, giriş ekranındakiyle aynı denetim.
std::string authorize(uid_t uid, const std::string &password)
{
    if (uid == 0)
        return {};
    const passwd *account = getpwuid(uid);
    if (!account || account->pw_uid < 1000)
        return "Bu kullanıcı paket kuramaz";
    if (!checkPassword(account->pw_name, password)) {
        // Şifre denemelerini yavaşlatır, servis tek istek işlediği için diğerleri de bekler
        sleep(2);
        return "Şifre yanlış";
    }
    return {};
}

} // namespace

int main()
{
    if (getuid() != 0) {
        fprintf(stderr, "ankaros-apps root olarak çalışmalıdır\n");
        return 1;
    }
    signal(SIGPIPE, SIG_IGN);
    const int epollFd = epoll_create1(EPOLL_CLOEXEC);
    ControlServer control;
    mkdir("/run/ankaros", 0755);
    const bool listening = control.listen(
        appsSocketPath, epollFd, [](const std::vector<std::string> &command, uid_t uid) {
            ControlServer::Reply reply;
            const std::string &name = command[0];
            if (name == "liste") {
                for (const InstalledApp &app : loadRegistry())
                    reply.lines.push_back(app.package + '\t' + app.name);
            } else if ((name == "kur" || name == "kaldir") && command.size() >= 2) {
                reply.error = authorize(uid, command.size() > 2 ? command[2] : "");
                if (reply.error.empty())
                    reply = name == "kur" ? install(command[1]) : remove(command[1]);
            } else {
                reply.error = "bilinmeyen komut: " + name;
            }
            return reply;
        });
    if (!listening) {
        fprintf(stderr, "soket açılamadı: %s: %s\n", appsSocketPath, strerror(errno));
        return 1;
    }

    for (;;) {
        epoll_event events[16];
        const int count = epoll_wait(epollFd, events, 16, -1);
        for (int i = 0; i < count; ++i)
            control.handle(events[i].data.fd);
    }
}
