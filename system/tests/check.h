#pragma once

// Bağımlılıksız küçük test yardımcısı. Her test dosyası bir main içerir, CTest çalıştırır.
#include <cstdio>
#include <string>

namespace check {

inline int failures = 0;

inline void report(bool ok, const char *expression, const char *file, int line)
{
    if (!ok) {
        ++failures;
        std::fprintf(stderr, "%s:%d: başarısız: %s\n", file, line, expression);
    }
}

inline int result(const char *name)
{
    if (failures == 0)
        std::printf("%s: bütün testler geçti\n", name);
    return failures == 0 ? 0 : 1;
}

} // namespace check

#define CHECK(expression)                                                                          \
    check::report(static_cast<bool>(expression), #expression, __FILE__, __LINE__)
#define CHECK_EQ(a, b) check::report((a) == (b), #a " == " #b, __FILE__, __LINE__)
