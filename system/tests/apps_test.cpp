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

static void findsIcons()
{
    char pattern[] = "/tmp/ankaros-simge-test-XXXXXX";
    CHECK(mkdtemp(pattern) != nullptr);
    const fs::path root = fs::path(pattern) / "kok";
    const std::string png = "\x89PNG\r\n\x1a\n....";
    const std::string svg = "<?xml version=\"1.0\"?><svg xmlns=\"http://www.w3.org/2000/svg\"/>";

    // SVG, PNG'den önce gelir
    write(root / "usr/share/icons/hicolor/scalable/apps/uyg.svg", svg);
    write(root / "usr/share/icons/hicolor/48x48/apps/uyg.png", png);
    auto icon = readIcon(root, "uyg");
    CHECK(icon && icon->extension == "svg" && icon->data == svg);

    // PNG'lerden en büyüğü seçilir
    write(root / "usr/share/icons/hicolor/48x48/apps/png.png", png + "48");
    write(root / "usr/share/icons/hicolor/256x256/apps/png.png", png + "256");
    write(root / "usr/share/icons/hicolor/128x128/apps/png.png", png + "128");
    icon = readIcon(root, "png");
    CHECK(icon && icon->extension == "png" && icon->data == png + "256");

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

    // Normal dosya olmalı ve sınırı aşmamalı
    fs::create_directories(root / "usr/share/pixmaps/klasor.png");
    CHECK(!readIcon(root, "klasor").has_value());
    write(root / "usr/share/pixmaps/buyuk.png", png + std::string(maxIconSize, 'x'));
    CHECK(!readIcon(root, "buyuk").has_value());

    std::error_code ignored;
    fs::remove_all(pattern, ignored);
}

int main()
{
    recognizesPackages();
    readsDesktopFiles();
    makesAppRecords();
    savesRecords();
    findsIcons();
    return check::result("apps_test");
}
