#pragma once

// Katmandaki uygulamanın simgesini bulup okur (docs/katmanlar.md). Katmanın içeriği güvenilmez:
// yol katman kökünün dışına çıkamaz, sembolik bağlantı izlenmez, dosya normal ve küçük olmalıdır.
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>

namespace ankaros {

// Simge dosyası için üst sınır
constexpr size_t maxIconSize = 1024 * 1024;

struct IconFile
{
    std::string extension; // "svg" ya da "png"
    std::string data;
};

// Icon= değerine göre simgeyi katman kökünden (root) okur. Değer mutlak yolsa katman köküne göre o
// dosya, değilse hicolor temasından (önce scalable svg, sonra en büyük png) ve pixmaps'ten ilk
// bulunan. Bulunamazsa ya da güvenli değilse boş döner.
std::optional<IconFile> readIcon(const std::string &root, std::string_view icon);

} // namespace ankaros
