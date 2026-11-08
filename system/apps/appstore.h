#pragma once

// Kurulan paketlerin kaydı ve .desktop dosyalarının okunması (docs/katmanlar.md). Dosya ve süreç
// işlemleri içermez, birim testleri Mac'te de çalışır.
#include <string>
#include <string_view>
#include <vector>

namespace ankaros {

enum class PackageKind
{
    Deb,
    Rpm,
    AppImage,
    Exe,
    Unknown
};

// Paket türü dosyanın adından anlaşılır
PackageKind packageKind(std::string_view fileName);

// Debian paket adı kuralı: küçük harf, rakam, + - . ve en az iki karakter
bool validPackageName(std::string_view name);

// Bir .desktop dosyasının [Desktop Entry] bölümü
struct DesktopEntry
{
    std::string name;
    std::string exec;
    std::string wmClass;
    std::string icon; // Icon=: simge adı ya da mutlak yol
    bool application = false;
    bool terminal = false;
    bool hidden = false; // NoDisplay ya da Hidden
};

// Name[tr] varsa Name'in önüne geçer
DesktopEntry parseDesktopEntry(std::string_view text);

// Exec satırındaki %f, %U gibi yer tutucuları kaldırır, %% yerine % yazar
std::string cleanExec(std::string_view exec);

// Kurulan bir paket. Başlat panelinde görünen bir uygulaması yoksa name boştur.
struct InstalledApp
{
    std::string package;
    std::string layer;
    std::string name;
    std::string command;
    std::vector<std::string> appIds;
    std::string icon; // veri diskindeki simge dosyası, yoksa boş
};

// .desktop dosyasından kayıt oluşturur. Başlat panelinde gösterilmeyecek bir dosyaysa (uygulama
// değil, gizli ya da terminalde çalışan) name boş kalır.
InstalledApp makeInstalledApp(const std::string &package, const std::string &layer,
                              const std::string &desktopId, const DesktopEntry &entry);

// Simge klasöründeki bir dosyanın adı olabilir mi: tek bileşen, "/" ve kontrol karakteri yok, "."
// ya da
// ".." değil
bool safeIconFile(std::string_view name);

// Paket ve .desktop kimliğinden simge dosyası adı. Farklı çiftler aynı adı üretmez.
std::string iconFileName(std::string_view package, std::string_view id, std::string_view extension);

// Kayıt dosyası: her satırda paket, katman, ad, komut, app_id'ler (boşlukla), simge yolu, alanlar
// sekmeyle ayrılır. Simge alanından önceki biçimde (5 alan) yazılmış kayıtlar da okunur.
std::string formatInstalled(const std::vector<InstalledApp> &apps);
std::vector<InstalledApp> parseInstalled(std::string_view text);

// AppCatalog'un okuduğu uygulama listesi (docs/uygulama-listesi.md)
std::string formatCatalog(const std::vector<InstalledApp> &apps);

} // namespace ankaros
