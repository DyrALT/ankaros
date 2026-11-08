#include "appicon.h"
#include "appstore.h"
#include "check.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <unistd.h>

using namespace ankaros;

static void recognizesPackages()
{
    CHECK(packageKind("code_arm64.deb") == PackageKind::Deb);
    CHECK(packageKind("/tmp/UYGULAMA.DEB") == PackageKind::Deb);
    CHECK(packageKind("a.rpm") == PackageKind::Rpm);
    CHECK(packageKind("a.AppImage") == PackageKind::AppImage);
    CHECK(packageKind("kurulum.exe") == PackageKind::Exe);
    CHECK(packageKind("belge.pdf") == PackageKind::Unknown);
    CHECK(packageKind("deb") == PackageKind::Unknown);

    CHECK(validPackageName("foot"));
    CHECK(validPackageName("libstdc++6"));
    CHECK(!validPackageName("a"));
    CHECK(!validPackageName("-x"));
    CHECK(!validPackageName("Foot"));
    CHECK(!validPackageName("../etc"));
    CHECK(!validPackageName("a b"));
}

static void readsDesktopFiles()
{
    const DesktopEntry entry = parseDesktopEntry("# yorum\r\n"
                                                 "[Desktop Entry]\r\n"
                                                 "Type=Application\r\n"
                                                 "Name=Text Editor\r\n"
                                                 "Name[tr]=Metin Düzenleyici\r\n"
                                                 "Exec=/usr/bin/code --new %U\r\n"
                                                 "StartupWMClass=Code\r\n"
                                                 "Icon=vscode\r\n"
                                                 "\r\n"
                                                 "[Desktop Action yeni]\r\n"
                                                 "Name=Yeni Pencere\r\n"
                                                 "Exec=/usr/bin/code --yeni\r\n");
    CHECK(entry.application);
    CHECK(!entry.terminal);
    CHECK(!entry.hidden);
    CHECK_EQ(entry.name, "Metin Düzenleyici");
    CHECK_EQ(entry.exec, "/usr/bin/code --new %U");
    CHECK_EQ(entry.wmClass, "Code");
    CHECK_EQ(entry.icon, "vscode");

    CHECK(parseDesktopEntry("[Desktop Entry]\nNoDisplay=true\n").hidden);
    CHECK(parseDesktopEntry("[Desktop Entry]\nHidden=true\n").hidden);
    CHECK(parseDesktopEntry("[Desktop Entry]\nTerminal=true\n").terminal);
    CHECK(!parseDesktopEntry("[Desktop Entry]\nType=Link\n").application);

    CHECK_EQ(cleanExec("foot"), "foot");
    CHECK_EQ(cleanExec("code %F"), "code");
    CHECK_EQ(cleanExec("uyg --a %u --b"), "uyg --a --b");
    CHECK_EQ(cleanExec("uyg --yuzde=100%%"), "uyg --yuzde=100%");
    CHECK_EQ(cleanExec(""), "");
}

static void makesAppRecords()
{
    DesktopEntry entry;
    entry.application = true;
    entry.name = "Code";
    entry.exec = "code %F";
    entry.wmClass = "Code-Win";
    const InstalledApp app = makeInstalledApp("code", "debian", "code", entry);
    CHECK_EQ(app.name, "Code");
    CHECK_EQ(app.command, "ankaros-katman debian -- code");
    CHECK_EQ(app.appIds.size(), 2u);
    CHECK_EQ(app.appIds[1], "Code-Win");

    // Başlat panelinde gösterilmeyenler paketi kayıtlı tutar, adı boş kalır
    DesktopEntry terminal = entry;
    terminal.terminal = true;
    CHECK(makeInstalledApp("code", "debian", "code", terminal).name.empty());
    DesktopEntry noExec = entry;
    noExec.exec = "%U";
    CHECK(makeInstalledApp("code", "debian", "code", noExec).name.empty());
    DesktopEntry notApp = entry;
    notApp.application = false;
    CHECK_EQ(makeInstalledApp("code", "debian", "code", notApp).package, "code");
}

static void savesRecords()
{
    InstalledApp app{"foot",           "debian", "Foot", "ankaros-katman debian -- foot",
                     {"foot", "Foot"}, ""};
    InstalledApp library{"kitaplik", "debian", "", "", {}, ""};
    const auto parsed = parseInstalled(formatInstalled({app, library}));
    CHECK_EQ(parsed.size(), 2u);
    CHECK_EQ(parsed[0].command, "ankaros-katman debian -- foot");
    CHECK_EQ(parsed[0].appIds.size(), 2u);
    CHECK_EQ(parsed[1].package, "kitaplik");
    CHECK(parsed[1].name.empty());
    CHECK(parsed[1].appIds.empty());
    CHECK(parseInstalled("").empty());
    CHECK(parseInstalled("bozuk satır\n").empty());
    // Simge alanı yokken yazılmış eski kayıt da okunur
    const auto old = parseInstalled("foot\tdebian\tFoot\tkomut\tfoot\n");
    CHECK_EQ(old.size(), 1u);
    CHECK(old[0].icon.empty());
    InstalledApp withIcon = app;
    withIcon.icon = "/var/lib/ankaros/simgeler/foot_foot.svg";
    CHECK_EQ(parseInstalled(formatInstalled({withIcon}))[0].icon, withIcon.icon);
    CHECK(formatCatalog({withIcon}).find("\"icon\": \"/var/lib/ankaros/simgeler/foot_foot.svg\"") !=
          std::string::npos);
    CHECK(formatCatalog({app}).find("icon") == std::string::npos);

    // Adı olmayan paket listeye girmez, özel karakterler kaçırılır
    InstalledApp quoted{"q", "debian", "A \"B\" \\C", "k", {"id"}, ""};
    CHECK_EQ(formatCatalog({library}), "{\n    \"apps\": []\n}\n");
    CHECK_EQ(formatCatalog({library, quoted}),
             "{\n    \"apps\": [\n        { \"name\": \"A \\\"B\\\" \\\\C\", \"command\": \"k\", "
             "\"appIds\": [\"id\"] }\n    ]\n}\n");
}

namespace fs = std::filesystem;

static void write(const fs::path &path, const std::string &data)
{
    fs::create_directories(path.parent_path());
    std::ofstream(path, std::ios::binary) << data;
}

// Geçerli bir PNG başlığı (imza + IHDR) ve verilen boyut
static std::string pngHeader(unsigned width = 64, unsigned height = 64)
{
    std::string data = "\x89PNG\r\n\x1a\n";
    data += std::string("\0\0\0\x0d", 4) + "IHDR";
    for (const unsigned value : {width, height}) {
        for (int shift = 24; shift >= 0; shift -= 8)
            data += static_cast<char>((value >> shift) & 0xff);
    }
    return data + std::string(8, '\0');
}

static void findsIcons()
{
    char pattern[] = "/tmp/ankaros-simge-test-XXXXXX";
    CHECK(mkdtemp(pattern) != nullptr);
    const fs::path root = fs::path(pattern) / "kok";
    const std::string png = pngHeader();
    const std::string svg = "<?xml version=\"1.0\"?><svg xmlns=\"http://www.w3.org/2000/svg\"/>";

    // SVG, PNG'den önce gelir
    write(root / "usr/share/icons/hicolor/scalable/apps/uyg.svg", svg);
    write(root / "usr/share/icons/hicolor/48x48/apps/uyg.png", png);
    auto icon = readIcon(root, "uyg");
    CHECK(icon && icon->extension == "svg" && icon->data == svg);

    // PNG'lerden en büyüğü seçilir
    write(root / "usr/share/icons/hicolor/48x48/apps/png.png", pngHeader(48, 48));
    write(root / "usr/share/icons/hicolor/256x256/apps/png.png", pngHeader(256, 256));
    write(root / "usr/share/icons/hicolor/128x128/apps/png.png", pngHeader(128, 128));
    icon = readIcon(root, "png");
    CHECK(icon && icon->extension == "png" && icon->data == pngHeader(256, 256));

    // En son pixmaps denenir
    write(root / "usr/share/pixmaps/eski.png", png);
    CHECK(readIcon(root, "eski").has_value());

    // Mutlak yol katman köküne göre çözülür
    write(root / "opt/uyg/logo.png", png);
    icon = readIcon(root, "/opt/uyg/logo.png");
    CHECK(icon && icon->extension == "png");

    // Simge yoksa, tür yanlışsa ya da içerik resim değilse bulunamaz
    CHECK(!readIcon(root, "yok").has_value());
    CHECK(!readIcon(root, "").has_value());
    write(root / "usr/share/pixmaps/sahte.png", "resim degil");
    CHECK(!readIcon(root, "sahte").has_value());
    write(root / "opt/uyg/logo.xpm", png);
    CHECK(!readIcon(root, "/opt/uyg/logo.xpm").has_value());

    // Katman kökünün dışına çıkış reddedilir
    write(fs::path(pattern) / "gizli.png", png);
    CHECK(!readIcon(root, "/../gizli.png").has_value());
    CHECK(!readIcon(root, "/opt/../../gizli.png").has_value());
    CHECK(!readIcon(root, "../gizli").has_value());
    CHECK(!readIcon(root, "..").has_value());

    // Sembolik bağlantı izlenmez: dosya da, klasör de
    fs::create_symlink(fs::path(pattern) / "gizli.png", root / "usr/share/pixmaps/baglanti.png");
    CHECK(!readIcon(root, "baglanti").has_value());
    fs::create_symlink(pattern, root / "usr/share/kacis");
    CHECK(!readIcon(root, "/usr/share/kacis/gizli.png").has_value());

    // NUL ile yol kaçışı: "..\0x" karşılaştırmayı geçip ".." olarak açılmasın
    CHECK(!readIcon(root, std::string("/..\0a/..\0a/gizli.png", 21)).has_value());
    CHECK(!readIcon(root, std::string("/opt/..\0a/..\0a/gizli.png", 25)).has_value());
    CHECK(!readIcon(root, std::string("uyg\0x", 5)).has_value());
    CHECK(!readIcon(root, "uy\ng").has_value());
    CHECK(!readRootFile(root, std::string("/opt/uyg/logo.png\0.x", 20), maxIconSize).has_value());
    CHECK(readRootFile(root, "//opt///uyg/./logo.png", maxIconSize) == std::nullopt);
    CHECK(readRootFile(root, "//opt///uyg/logo.png", maxIconSize).has_value());

    // SVG: DTD, varlık ve dış başvurular reddedilir, "#" ile başlayanlar kabul
    const auto svgWith = [&](const std::string &body) {
        write(root / "usr/share/pixmaps/s.svg",
              "<svg xmlns=\"http://www.w3.org/2000/svg\">" + body + "</svg>");
        return readIcon(root, "s").has_value();
    };
    CHECK(svgWith("<use href=\"#a\"/>"));
    CHECK(svgWith("<use xlink:href='#a'/>"));
    CHECK(!svgWith("<image href=\"file:///etc/passwd\"/>"));
    CHECK(!svgWith("<image xlink:href = \"http://x/y.png\"/>"));
    CHECK(!svgWith("<image href=\"baska.png\"/>"));
    CHECK(!svgWith("<!DOCTYPE svg><g/>"));
    CHECK(!svgWith("<!ENTITY x SYSTEM \"file:///etc/passwd\">"));
    CHECK(!svgWith("<style>@import url(http://x);</style>"));
    CHECK(!svgWith("<rect style=\"fill:url(http://x/a)\"/>"));
    CHECK(svgWith("<rect style=\"fill:url(#g)\"/>"));

    // PNG boyutu sınırı: çok büyük olan atlanır, küçüğü seçilir
    write(root / "usr/share/icons/hicolor/512x512/apps/dev.png", pngHeader(60000, 60000));
    write(root / "usr/share/icons/hicolor/64x64/apps/dev.png", pngHeader(64, 64));
    icon = readIcon(root, "dev");
    CHECK(icon && icon->data == pngHeader(64, 64));
    write(root / "usr/share/pixmaps/dev2.png", pngHeader(0, 10));
    CHECK(!readIcon(root, "dev2").has_value());

    // .desktop gibi dosyalar için bağlantı izlenmez, boyut sınırı uygulanır
    write(root / "usr/share/applications/a.desktop", "[Desktop Entry]\n");
    CHECK(readRootFile(root, "/usr/share/applications/a.desktop", 64 * 1024).has_value());
    CHECK(!readRootFile(root, "/usr/share/applications/a.desktop", 4).has_value());
    fs::create_symlink("/dev/zero", root / "usr/share/applications/zero.desktop");
    CHECK(!readRootFile(root, "/usr/share/applications/zero.desktop", 64 * 1024).has_value());
    fs::create_symlink(fs::path(pattern) / "gizli.png", root / "usr/share/applications/h.desktop");
    CHECK(!readRootFile(root, "/usr/share/applications/h.desktop", 64 * 1024).has_value());

    // Normal dosya olmalı ve sınırı aşmamalı
    fs::create_directories(root / "usr/share/pixmaps/klasor.png");
    CHECK(!readIcon(root, "klasor").has_value());
    write(root / "usr/share/pixmaps/buyuk.png", png + std::string(maxIconSize, 'x'));
    CHECK(!readIcon(root, "buyuk").has_value());

    std::error_code ignored;
    fs::remove_all(pattern, ignored);
}

static void namesIconFiles()
{
    CHECK(safeIconFile("foot_foot-0123456789ab.svg"));
    CHECK(!safeIconFile(""));
    CHECK(!safeIconFile("."));
    CHECK(!safeIconFile(".."));
    CHECK(!safeIconFile("../x"));
    CHECK(!safeIconFile("a/b"));
    CHECK(!safeIconFile(std::string("a\0b", 3)));
    CHECK(!safeIconFile("a\nb"));

    // Temizlenince aynı olan çiftler farklı ad alır
    CHECK(iconFileName("a", "b c", "png") != iconFileName("a", "b_c", "png"));
    CHECK(iconFileName("ab", "c", "png") != iconFileName("a", "bc", "png"));
    CHECK_EQ(iconFileName("foot", "foot", "svg"), iconFileName("foot", "foot", "svg"));
    CHECK(safeIconFile(iconFileName("p", "../../etc/x y\n", "png")));
    CHECK(safeIconFile(iconFileName("p", std::string(500, 'a'), "png")));

    // NUL içeren .desktop satırı atlanır
    const DesktopEntry entry =
        parseDesktopEntry(std::string("[Desktop Entry]\nIcon=/..\0a/x.png\nName=Uyg\n", 43));
    CHECK(entry.icon.empty());
    CHECK_EQ(entry.name, "Uyg");
}

int main()
{
    recognizesPackages();
    readsDesktopFiles();
    makesAppRecords();
    savesRecords();
    findsIcons();
    namesIconFiles();
    return check::result("apps_test");
}
