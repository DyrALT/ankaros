// ankaros-init: ankaros'un init sistemi (PID 1). Ayrıntılar: docs/init.md
//
// Açılış: dosya sistemleri bağlanır, /etc/ankaros/services/*.service okunur, servisler bağımlılık
// sırasıyla başlatılır. Sonra ana döngü süreç bitişlerini (SIGCHLD), kapatma sinyallerini ve
// ankarosctl'den gelen komutları bekler.
#include "control.h"
#include "log.h"
#include "protocol.h"
#include "supervisor.h"
#include "system.h"

#include <cerrno>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <optional>
#include <sys/epoll.h>
#include <sys/reboot.h>
#include <sys/signalfd.h>
#include <unistd.h>

using namespace ankaros;

namespace {

constexpr const char *servicesDirectory = "/etc/ankaros/services";

std::optional<PowerAction> requestedAction;

// BusyBox'ın halt, poweroff ve reboot komutları PID 1'e bu sinyalleri gönderir.
// SIGINT: Ctrl-Alt-Del (RB_DISABLE_CAD ile çekirdek yeniden başlatmak yerine sinyal gönderir).
std::optional<PowerAction> actionForSignal(int signal)
{
    switch (signal) {
    case SIGUSR1:
        return PowerAction::Halt;
    case SIGUSR2:
        return PowerAction::Poweroff;
    case SIGTERM:
    case SIGINT:
        return PowerAction::Reboot;
    default:
        return std::nullopt;
    }
}

ControlServer::Reply handleCommand(Supervisor &supervisor, const std::vector<std::string> &command,
                                   uid_t uid)
{
    ControlServer::Reply reply;
    const std::string &name = command[0];
    const std::string argument = command.size() > 1 ? command[1] : std::string();

    if (name == "status") {
        reply.lines = supervisor.statusLines();
    } else if (name == "boot") {
        reply.lines = supervisor.bootLines();
    } else if (name == "mark" && command.size() == 2) {
        supervisor.mark(argument);
    } else if (uid != 0) {
        reply.error = "bu komut için root olmalısınız";
    } else if ((name == "start" || name == "stop" || name == "restart") && command.size() == 2) {
        std::string error;
        const bool ok = name == "start"  ? supervisor.start(argument, error)
                        : name == "stop" ? supervisor.stop(argument, error)
                                         : supervisor.restart(argument, error);
        if (!ok)
            reply.error = error;
    } else if (name == "poweroff" && command.size() == 1) {
        requestedAction = PowerAction::Poweroff;
    } else if (name == "reboot" && command.size() == 1) {
        requestedAction = PowerAction::Reboot;
    } else {
        reply.error = "bilinmeyen komut: " + name;
    }
    return reply;
}

} // namespace

int main()
{
    if (getpid() != 1) {
        fprintf(stderr, "ankaros-init yalnızca PID 1 olarak çalışır. Denetim için: ankarosctl\n");
        return 1;
    }
    const double startedAt = log::now();
    log::write("ankaros-init başladı");
    prepareSystem();
    log::openFile("/var/log/ankaros-init.log");
    log::write("çekirdek açılışı %.3f sn sürdü", startedAt);

    // Sinyaller signalfd ile ana döngüde işlenir. Çocuk süreçler maskeyi exec'ten önce sıfırlar.
    sigset_t signals;
    sigemptyset(&signals);
    for (int signal : {SIGCHLD, SIGTERM, SIGINT, SIGUSR1, SIGUSR2, SIGHUP, SIGPIPE})
        sigaddset(&signals, signal);
    sigprocmask(SIG_BLOCK, &signals, nullptr);
    const int signalFd = signalfd(-1, &signals, SFD_NONBLOCK | SFD_CLOEXEC);
    reboot(RB_DISABLE_CAD);

    Supervisor supervisor;
    supervisor.load(servicesDirectory);
    if (supervisor.empty()) {
        log::write("hiç servis yok, seri konsolda giriş açılıyor");
        ServiceDefinition getty;
        getty.name = "getty";
        getty.restart = RestartPolicy::Always;
        getty.commands = {{"/sbin/getty", "-L", "console", "0", "vt100"}};
        supervisor.add(std::move(getty));
    }

    const int epollFd = epoll_create1(EPOLL_CLOEXEC);
    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = signalFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, signalFd, &event);

    ControlServer control;
    if (!control.listen(initSocketPath, epollFd,
                        [&supervisor](const std::vector<std::string> &command, uid_t uid) {
                            return handleCommand(supervisor, command, uid);
                        }))
        log::write("denetim soketi açılamadı: %s: %s", initSocketPath, strerror(errno));

    supervisor.update();
    while (!requestedAction) {
        const double timeout = supervisor.timeout();
        epoll_event events[16];
        const int count = epoll_wait(epollFd, events, 16,
                                     timeout < 0 ? -1 : static_cast<int>(timeout * 1000) + 1);
        if (count < 0 && errno != EINTR)
            log::write("epoll_wait: %s", strerror(errno));

        for (int i = 0; i < count; ++i) {
            const int fd = events[i].data.fd;
            if (fd != signalFd) {
                control.handle(fd);
                continue;
            }
            signalfd_siginfo info{};
            while (read(signalFd, &info, sizeof info) == sizeof info) {
                if (info.ssi_signo == SIGCHLD) {
                    reapChildren(
                        [&](pid_t pid, int status) { supervisor.childExited(pid, status); });
                } else if (const auto action = actionForSignal(static_cast<int>(info.ssi_signo))) {
                    requestedAction = action;
                }
            }
        }
        supervisor.update();
    }

    log::write("%s başladı, servisler durduruluyor", toString(*requestedAction));
    supervisor.stopAll(5.0);
    finishShutdown(*requestedAction);
}
