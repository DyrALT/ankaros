#include "appstore.h"
#include "check.h"

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
    InstalledApp app{"foot", "debian", "Foot", "ankaros-katman debian -- foot", {"foot", "Foot"}};
    InstalledApp library{"kitaplik", "debian", "", "", {}};
    const auto parsed = parseInstalled(formatInstalled({app, library}));
    CHECK_EQ(parsed.size(), 2u);
    CHECK_EQ(parsed[0].command, "ankaros-katman debian -- foot");
    CHECK_EQ(parsed[0].appIds.size(), 2u);
    CHECK_EQ(parsed[1].package, "kitaplik");
    CHECK(parsed[1].name.empty());
    CHECK(parsed[1].appIds.empty());
    CHECK(parseInstalled("").empty());
    CHECK(parseInstalled("bozuk satır\n").empty());

    // Adı olmayan paket listeye girmez, özel karakterler kaçırılır
    InstalledApp quoted{"q", "debian", "A \"B\" \\C", "k", {"id"}};
    CHECK_EQ(formatCatalog({library}), "{\n    \"apps\": []\n}\n");
    CHECK_EQ(formatCatalog({library, quoted}),
             "{\n    \"apps\": [\n        { \"name\": \"A \\\"B\\\" \\\\C\", \"command\": \"k\", "
             "\"appIds\": [\"id\"] }\n    ]\n}\n");
}

int main()
{
    recognizesPackages();
    readsDesktopFiles();
    makesAppRecords();
    savesRecords();
    return check::result("apps_test");
}
