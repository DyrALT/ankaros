// wpctl çıktısının çözümlenmesi (wpctl.h)
#include "wpctl.h"

#include <clocale>
#include <cstdio>

namespace {
int failures = 0;
void check(bool ok, const char *what, int line)
{
    if (!ok) {
        ++failures;
        std::fprintf(stderr, "wpctl_test.cpp:%d: başarısız: %s\n", line, what);
    }
}
} // namespace
#define CHECK(expr) check((expr), #expr, __LINE__)

int main()
{
    // Ondalık ayıracı virgül olan dillerde de çalışmalı (imajda LANG=tr_TR.UTF-8)
    std::setlocale(LC_ALL, "tr_TR.UTF-8");
    wpctl::Volume v = wpctl::parseVolume("Volume: 0.45\n");
    CHECK(v.valid);
    CHECK(v.percent == 45);
    CHECK(!v.muted);

    v = wpctl::parseVolume("Volume: 1.00 [MUTED]\n");
    CHECK(v.valid);
    CHECK(v.percent == 100);
    CHECK(v.muted);

    // Yuvarlama ve sınır
    CHECK(wpctl::parseVolume("Volume: 0.335\n").percent == 34);
    CHECK(wpctl::parseVolume("Volume: 0.00\n").percent == 0);
    CHECK(wpctl::parseVolume("Volume: 1.50\n").percent == 150);
    CHECK(wpctl::parseVolume("Volume: 3.00\n").percent == 150);

    // Geçersiz ya da boş çıktı: varsayılan çıkış yok, wpctl bulunamadı, bozuk satır
    CHECK(!wpctl::parseVolume("").valid);
    CHECK(!wpctl::parseVolume("Could not find default node").valid);
    CHECK(!wpctl::parseVolume("Volume: abc\n").valid);
    CHECK(!wpctl::parseVolume("Volume: -0.5\n").valid);
    CHECK(!wpctl::parseVolume("Volume: 99\n").valid);

    CHECK(wpctl::formatLevel(0) == "0.00");
    CHECK(wpctl::formatLevel(45) == "0.45");
    CHECK(wpctl::formatLevel(100) == "1.00");
    CHECK(wpctl::formatLevel(150) == "1.50");
    CHECK(wpctl::formatLevel(-5) == "0.00");
    CHECK(wpctl::formatLevel(999) == "1.50");

    // Simge kademeleri
    CHECK(std::string(wpctl::iconName(50, true)) == "volumeMute");
    CHECK(std::string(wpctl::iconName(0, false)) == "volumeMute");
    CHECK(std::string(wpctl::iconName(1, false)) == "volume1");
    CHECK(std::string(wpctl::iconName(33, false)) == "volume1");
    CHECK(std::string(wpctl::iconName(34, false)) == "volume2");
    CHECK(std::string(wpctl::iconName(66, false)) == "volume2");
    CHECK(std::string(wpctl::iconName(67, false)) == "volume3");
    CHECK(std::string(wpctl::iconName(100, false)) == "volume3");
    CHECK(std::string(wpctl::iconName(150, false)) == "volume3");

    if (failures == 0)
        std::printf("wpctl_test: bütün testler geçti\n");
    return failures == 0 ? 0 : 1;
}
