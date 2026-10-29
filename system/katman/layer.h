#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace ankaros {

// Uyumluluk katmanlarının (karar 0016) durduğu klasör, veri diskinde (karar 0017). Her katman
// <ad>/kok altında başka bir dağıtımın kök dosya sistemidir.
inline constexpr std::string_view layersDirectory = "/var/lib/ankaros/katmanlar";

// Katman adı: küçük harf, rakam ve "-", en çok 32 karakter (ör. "debian"). Yol olarak kullanıldığı
// için
// "/" ve ".." kabul edilmez.
bool validLayerName(std::string_view name);
std::string layerRoot(std::string_view name);

// Katmanın içine ankaros'tan bağlanan bir yol
struct SharedPath
{
    std::string source;
    // Katmanın içindeki yer
    std::string target;
    // Dosya mı (ör. /etc/resolv.conf), klasör mü
    bool file = false;
};

// Katmandaki uygulamanın ankaros'la paylaştığı yollar: aygıtlar, /proc, /sys, /tmp, ağ adı çözümü
// ve kullanıcı için ev klasörü ile çalışma zamanı klasörü (Wayland soketi). Root için (apt)
// yalnızca ilk kısım. home ve runtimeDir boşsa atlanır. /home ve /run katmanda boş birer tmpfs
// olarak açılır, bu klasörlerin altındaki yerler orada oluşturulur.
std::vector<SharedPath> sharedPaths(const std::string &home, const std::string &runtimeDir);

} // namespace ankaros
