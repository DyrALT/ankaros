#include "log.h"

#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <fcntl.h>
#include <string>
#include <unistd.h>

namespace ankaros::log {

namespace {

int fileFd = -1;
std::string pending;

void writeAll(int fd, const char *data, size_t size)
{
    while (size > 0) {
        const ssize_t written = ::write(fd, data, size);
        if (written <= 0)
            return;
        data += written;
        size -= static_cast<size_t>(written);
    }
}

} // namespace

double now()
{
    timespec time{};
    clock_gettime(CLOCK_BOOTTIME, &time);
    return static_cast<double>(time.tv_sec) + static_cast<double>(time.tv_nsec) / 1e9;
}

void openFile(const char *path)
{
    fileFd = ::open(path, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
    if (fileFd >= 0) {
        writeAll(fileFd, pending.data(), pending.size());
        pending.clear();
        pending.shrink_to_fit();
    }
}

void write(const char *format, ...)
{
    char message[1024];
    va_list arguments;
    va_start(arguments, format);
    vsnprintf(message, sizeof message, format, arguments);
    va_end(arguments);

    char line[1100];
    const int length = snprintf(line, sizeof line, "[%8.3f] %s\n", now(), message);
    if (length <= 0)
        return;
    const size_t size = std::min(static_cast<size_t>(length), sizeof line - 1);
    writeAll(STDOUT_FILENO, line, size);
    if (fileFd >= 0)
        writeAll(fileFd, line, size);
    else
        pending.append(line, size);
}

} // namespace ankaros::log
