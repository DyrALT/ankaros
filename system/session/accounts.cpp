#include "accounts.h"

#include "config.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef __linux__
#include <crypt.h>
#include <shadow.h>
#endif

namespace ankaros {

bool replaceShadowHash(std::string &shadow, std::string_view user, std::string_view hash)
{
    const std::string prefix = std::string(user) + ":";
    for (size_t start = 0; start < shadow.size();) {
        const size_t end = std::min(shadow.find('\n', start), shadow.size());
        if (shadow.compare(start, prefix.size(), prefix) == 0) {
            const size_t hashStart = start + prefix.size();
            const size_t hashEnd = std::min(shadow.find(':', hashStart), end);
            shadow.replace(hashStart, hashEnd - hashStart, hash);
            return true;
        }
        start = end + 1;
    }
    return false;
}

#ifdef __linux__

bool matchesHash(const std::string &password, const std::string &hash)
{
    if (hash.empty() || hash[0] == '!' || hash[0] == '*')
        return false;
    crypt_data data{};
    const char *result = crypt_r(password.c_str(), hash.c_str(), &data);
    // Hatada crypt_r "*" ile başlayan bir değer döner, o da eşleşmez
    return result && result[0] != '*' && hash == result;
}

std::string makeHash(const std::string &password)
{
    char salt[CRYPT_GENSALT_OUTPUT_SIZE];
    if (!crypt_gensalt_rn("$6$", 0, nullptr, 0, salt, sizeof salt))
        return {};
    crypt_data data{};
    const char *result = crypt_r(password.c_str(), salt, &data);
    return result && result[0] != '*' ? std::string(result) : std::string();
}

bool checkPassword(const std::string &user, const std::string &password)
{
    const spwd *entry = getspnam(user.c_str());
    return entry && entry->sp_pwdp && matchesHash(password, entry->sp_pwdp);
}

bool setPassword(const std::string &user, const std::string &password, std::string &error)
{
    constexpr const char *path = "/etc/shadow";
    constexpr const char *temporary = "/etc/shadow+";
    std::string shadow;
    if (!readFile(path, shadow)) {
        error = "şifre dosyası okunamadı";
        return false;
    }
    const std::string hash = makeHash(password);
    if (hash.empty() || !replaceShadowHash(shadow, user, hash)) {
        error = "şifre değiştirilemedi";
        return false;
    }
    struct stat info = {};
    stat(path, &info);
    const int fd = open(temporary, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0600);
    const bool written =
        fd >= 0 && write(fd, shadow.data(), shadow.size()) == static_cast<ssize_t>(shadow.size()) &&
        fchown(fd, info.st_uid, info.st_gid) == 0 && fchmod(fd, info.st_mode & 07777) == 0 &&
        fsync(fd) == 0;
    if (fd >= 0)
        close(fd);
    if (!written || rename(temporary, path) != 0) {
        error = std::string("şifre dosyası yazılamadı: ") + strerror(errno);
        unlink(temporary);
        return false;
    }
    return true;
}

#endif

} // namespace ankaros
