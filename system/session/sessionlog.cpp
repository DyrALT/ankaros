#include "sessionlog.h"

#include <cstdio>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

namespace ankaros {

namespace {

// Satır sonu gelmeyen çıktı bundan uzunsa yine de yazılır, bellekte birikmez
constexpr size_t maxLine = 4096;

} // namespace

SessionLog::SessionLog(std::string path, size_t maxSize, Clock clock)
    : m_path(std::move(path))
    , m_maxSize(maxSize)
    , m_clock(std::move(clock))
{
    open();
}

SessionLog::~SessionLog()
{
    if (m_fd >= 0)
        close(m_fd);
}

void SessionLog::open()
{
    m_fd = ::open(m_path.c_str(), O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC | O_NOFOLLOW, 0644);
    struct stat info;
    m_size = m_fd >= 0 && fstat(m_fd, &info) == 0 ? static_cast<size_t>(info.st_size) : 0;
}

void SessionLog::rotate()
{
    if (m_fd >= 0)
        close(m_fd);
    rename(m_path.c_str(), (m_path + ".1").c_str());
    open();
}

void SessionLog::writeLine(const std::string &line, bool stamped)
{
    std::string text;
    if (stamped) {
        char stamp[32];
        snprintf(stamp, sizeof stamp, "[%8.3f] ", m_clock());
        text = stamp;
    }
    text += line;
    text += '\n';
    if (m_size + text.size() > m_maxSize)
        rotate();
    if (m_fd < 0)
        return;
    const ssize_t written = write(m_fd, text.data(), text.size());
    if (written > 0)
        m_size += static_cast<size_t>(written);
}

void SessionLog::append(const char *data, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        if (data[i] == '\n') {
            writeLine(m_partial, true);
            m_partial.clear();
        } else {
            m_partial += data[i];
            if (m_partial.size() >= maxLine) {
                writeLine(m_partial, true);
                m_partial.clear();
            }
        }
    }
}

void SessionLog::flush()
{
    if (!m_partial.empty()) {
        writeLine(m_partial, true);
        m_partial.clear();
    }
}

void SessionLog::note(const std::string &line)
{
    flush();
    writeLine(line, false);
}

} // namespace ankaros
