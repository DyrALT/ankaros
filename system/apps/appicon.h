#pragma once

// Katmandaki uygulamanın simgesini bulup okur (docs/katmanlar.md). Katmanın içeriği güvenilmez:
// yol katman kökünün dışına çıkamaz, sembolik bağlantı izlenmez, dosya normal ve küçük olmalıdır.
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace ankaros {

// Simge dosyası için üst sınır
constexpr size_t maxIconSize = 1024 * 1024;
// PNG'nin genişlik ve yüksekliği için üst sınır (piksel)
constexpr uint32_t maxIconDimension = 1024;

struct IconFile
{
    std::string extension; // "svg" ya da "png"
    std::string data;
};

// Katman kökünden (root) bir dosyayı güvenle okur: yol bileşen bileşen openat ile açılır, hiçbir
// bileşen sembolik bağlantı olamaz, ".", ".." ve kontrol karakteri (NUL dahil) reddedilir, dosya
// normal olmalı ve maxSize'ı aşmamalı. Başarısızsa boş döner.
std::optional<std::string> readRootFile(const std::string &root, std::string_view path,
                                        size_t maxSize);

// Icon= değerine göre simgeyi katman kökünden (root) okur. Değer mutlak yolsa katman köküne göre o
// dosya, değilse hicolor temasından (önce scalable svg, sonra en büyük png) ve pixmaps'ten ilk
// bulunan. Bulunamazsa ya da güvenli değilse boş döner.
std::optional<IconFile> readIcon(const std::string &root, std::string_view icon);

} // namespace ankaros
