#pragma once

#include <cstdio>
#include <string>
#include <string_view>

// wpctl (WirePlumber) çıktısını çözümler. Qt'ye bağlı değil, birim testi var (wpctl_test.cpp).
namespace wpctl {

struct Volume
{
    bool valid = false;
    int percent = 0; // 0-150, 100 = ses düzeyi 1.0
    bool muted = false;
};

// "wpctl get-volume @DEFAULT_AUDIO_SINK@" çıktısı: "Volume: 0.45" ya da "Volume: 0.45 [MUTED]".
// Varsayılan çıkış yoksa wpctl hata verir ve çıktı bu biçimde olmaz: valid false kalır.
inline Volume parseVolume(std::string_view text)
{
    constexpr std::string_view prefix = "Volume:";
    Volume volume;
    const size_t start = text.find(prefix);
    if (start == std::string_view::npos)
        return volume;
    // strtod kullanılmaz: tr_TR gibi dillerde ondalık ayıracı virgüldür, "0.45"yi "0" diye okurdu
    // (Qt uygulamaları setlocale(LC_ALL, "") çağırır)
    size_t i = start + prefix.size();
    while (i < text.size() && text[i] == ' ')
        ++i;
    int whole = 0;
    size_t digits = 0;
    for (; i < text.size() && text[i] >= '0' && text[i] <= '9'; ++i, ++digits)
        whole = whole * 10 + (text[i] - '0');
    if (digits == 0 || digits > 2 || whole > 10)
        return volume;
    int hundredths = 0;
    if (i < text.size() && text[i] == '.') {
        ++i;
        int scale = 10;
        double rest = 0;
        for (; i < text.size() && text[i] >= '0' && text[i] <= '9'; ++i) {
            rest += (text[i] - '0') / static_cast<double>(scale);
            scale *= 10;
        }
        hundredths = static_cast<int>(rest * 100 + 0.5);
    }
    volume.valid = true;
    volume.percent = whole * 100 + hundredths;
    if (volume.percent > 150)
        volume.percent = 150;
    const std::string_view tail = text.substr(i);
    volume.muted = tail.find("[MUTED]") != std::string_view::npos;
    return volume;
}

// "wpctl set-volume" için 0-150 arası yüzdeyi "0.45" biçimine çevirir
inline std::string formatLevel(int percent)
{
    if (percent < 0)
        percent = 0;
    if (percent > 150)
        percent = 150;
    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%d.%02d", percent / 100, percent % 100);
    return buffer;
}

// Ses simgesinin adı (UI/IconData.js): sessizde ve ses 0'dayken "volumeMute" (hoparlör ve çarpı),
// 1-33 bir, 34-66 iki, 67 ve üstü üç dalga. Windows 11'in ses simgesiyle aynı kademeler.
inline const char *iconName(int percent, bool muted)
{
    if (muted || percent <= 0)
        return "volumeMute";
    if (percent < 34)
        return "volume1";
    if (percent < 67)
        return "volume2";
    return "volume3";
}

} // namespace wpctl
