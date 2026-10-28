#include "ptyprocess.h"

#include <QFileInfo>
#include <QSocketNotifier>

#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <pwd.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>

#ifdef Q_OS_MACOS
#include <libproc.h>
#include <util.h>
#else
#include <pty.h>
#endif

PtyProcess::PtyProcess(QObject *parent)
    : QObject(parent)
{}

PtyProcess::~PtyProcess()
{
    if (m_pid > 0) {
        // Kabuğa terminalin kapandığını bildir (sekme kapatıldı)
        kill(m_pid, SIGHUP);
        waitpid(m_pid, nullptr, WNOHANG);
    }
    if (m_fd >= 0)
        close(m_fd);
}

bool PtyProcess::start(int rows, int columns)
{
    winsize size{};
    size.ws_row = static_cast<unsigned short>(rows);
    size.ws_col = static_cast<unsigned short>(columns);

    // Kabuk: SHELL ortam değişkeni (oturum yöneticisi verir), yoksa hesabın kabuğu
    const passwd *account = getpwuid(getuid());
    const char *shell = getenv("SHELL");
    if (!shell || !*shell)
        shell = account && account->pw_shell && *account->pw_shell ? account->pw_shell : "/bin/sh";
    const char *home = account ? account->pw_dir : nullptr;

    m_pid = forkpty(&m_fd, nullptr, nullptr, &size);
    if (m_pid < 0) {
        qWarning("ankaros-terminal: forkpty: %s", strerror(errno));
        return false;
    }
    if (m_pid == 0) {
        // Çocuk süreç: giriş kabuğu ("-sh"), profil dosyalarını okur
        setenv("TERM", "xterm-256color", 1);
        setenv("COLORTERM", "truecolor", 1);
        if (home && chdir(home) == 0)
            setenv("PWD", home, 1);
        const QByteArray name =
            "-" + QFileInfo(QString::fromLocal8Bit(shell)).fileName().toLocal8Bit();
        execl(shell, name.constData(), static_cast<char *>(nullptr));
        _exit(127);
    }

    fcntl(m_fd, F_SETFL, fcntl(m_fd, F_GETFL) | O_NONBLOCK);
    fcntl(m_fd, F_SETFD, FD_CLOEXEC);
    m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &PtyProcess::read);
    return true;
}

void PtyProcess::read()
{
    char buffer[16384];
    QByteArray data;
    for (;;) {
        const ssize_t count = ::read(m_fd, buffer, sizeof buffer);
        if (count > 0) {
            data.append(buffer, count);
            // Çok büyük çıktıda arayüz donmasın, kalanı bir sonraki turda okunur
            if (data.size() >= 256 * 1024)
                break;
            continue;
        }
        if (count < 0 && (errno == EAGAIN || errno == EINTR))
            break;
        // Kabuk kapandı (Linux'ta EIO, macOS'ta 0)
        if (!data.isEmpty())
            emit received(data);
        m_notifier->setEnabled(false);
        if (m_pid > 0) {
            waitpid(m_pid, nullptr, 0);
            m_pid = -1;
        }
        emit finished();
        return;
    }
    if (!data.isEmpty())
        emit received(data);
}

void PtyProcess::write(const char *data, size_t size)
{
    while (size > 0 && m_fd >= 0) {
        const ssize_t written = ::write(m_fd, data, size);
        if (written < 0) {
            if (errno == EINTR || errno == EAGAIN)
                continue;
            return;
        }
        data += written;
        size -= size_t(written);
    }
}

void PtyProcess::resize(int rows, int columns)
{
    if (m_fd < 0)
        return;
    winsize size{};
    size.ws_row = static_cast<unsigned short>(rows);
    size.ws_col = static_cast<unsigned short>(columns);
    ioctl(m_fd, TIOCSWINSZ, &size);
}

QString PtyProcess::workingDirectory() const
{
    if (m_fd < 0)
        return {};
    pid_t process = tcgetpgrp(m_fd);
    if (process <= 0)
        process = m_pid;
#ifdef Q_OS_MACOS
    proc_vnodepathinfo info{};
    if (proc_pidinfo(process, PROC_PIDVNODEPATHINFO, 0, &info, sizeof info) <= 0)
        return {};
    return QString::fromLocal8Bit(info.pvi_cdir.vip_path);
#else
    char target[4096];
    const QByteArray link = QByteArray("/proc/") + QByteArray::number(process) + "/cwd";
    const ssize_t length = readlink(link.constData(), target, sizeof target - 1);
    return length > 0 ? QString::fromLocal8Bit(target, length) : QString();
#endif
}
