#include "accounts.h"
#include "check.h"
#include "control.h"
#include "sessionconfig.h"
#include "sessionlog.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <unistd.h>

using namespace ankaros;

static void replacesShadowHash()
{
    std::string shadow = "root::19000:0:99999:7:::\n"
                         "ankaros:$5$eski$hash:19000:0:99999:7:::\n"
                         "ankaros-login:*:19000:0:99999:7:::\n";
    CHECK(replaceShadowHash(shadow, "ankaros", "$6$yeni$hash"));
    CHECK_EQ(shadow, "root::19000:0:99999:7:::\n"
                     "ankaros:$6$yeni$hash:19000:0:99999:7:::\n"
                     "ankaros-login:*:19000:0:99999:7:::\n");
    // Ön eki aynı olan başka bir kullanıcı karışmaz
    CHECK(replaceShadowHash(shadow, "ankaros-login", "!"));
    CHECK(shadow.find("ankaros-login:!:19000") != std::string::npos);
    CHECK(shadow.find("ankaros:$6$yeni$hash") != std::string::npos);
    CHECK(!replaceShadowHash(shadow, "yok", "x"));
    // Son satırda satır sonu yoksa da çalışır
    std::string last = "a:x:1";
    CHECK(replaceShadowHash(last, "a", "y"));
    CHECK_EQ(last, "a:y:1");
}

static void readsAndWritesConfig()
{
    CHECK_EQ(parseSessionConfig("autologin = ankaros\n").autologin, "ankaros");
    CHECK_EQ(parseSessionConfig("# yok\n").autologin, "");
    CHECK_EQ(parseSessionConfig("autologin =\n").autologin, "");
    const SessionConfig config{"ankaros"};
    CHECK_EQ(parseSessionConfig(formatSessionConfig(config)).autologin, "ankaros");
    CHECK_EQ(parseSessionConfig(formatSessionConfig({})).autologin, "");
    // Ölçek: auto (0), 1 ya da 2; dosya yeniden yazılınca korunur
    CHECK(parseSessionConfig("autologin = ankaros\n").scale == 0);
    CHECK(parseSessionConfig("scale = auto\n").scale == 0);
    CHECK(parseSessionConfig("scale = 2\n").scale == 2);
    CHECK(parseSessionConfig("scale = 3\n").scale == 0);
    CHECK(parseSessionConfig(formatSessionConfig({"ankaros", 1})).scale == 1);
    CHECK(parseSessionConfig(formatSessionConfig({"ankaros", 0})).scale == 0);
}

static void forwardsAppearanceToGreeter()
{
    const std::string conf =
        "[General]\nwallpaper=gece\n"
        "[appearance]\ndarkMode=true\nwallpaper=kehribar\nwallpaperOnLock=true\n";
    const auto env = greeterAppearance(conf);
    CHECK_EQ(env.size(), 2u);
    if (env.size() != 2)
        return; // aşağıdaki indeksler sınırı aşmasın
    CHECK_EQ(env[0], "ANKAROS_DARK_MODE=true");
    CHECK_EQ(env[1], "ANKAROS_WALLPAPER=kehribar"); // başka bölümdeki değer karışmaz

    // Duvar kağıdı kilitte istenmiyorsa iletilmez, giriş ekranı varsayılanı kullanır
    CHECK_EQ(greeterAppearance("[appearance]\nwallpaper=gece\nwallpaperOnLock=false\n").size(), 0u);
    // Bilinmeyen değerler iletilmez
    CHECK_EQ(greeterAppearance("[appearance]\nwallpaper=../../x\ndarkMode=evet\n").size(), 0u);
    CHECK(greeterAppearance("").empty());
}

static void forwardsKeyboardLayout()
{
    const auto f = keyboardEnvironment("[appearance]\nkeyboardLayout=tr-f\n", false);
    CHECK_EQ(f.size(), 2u);
    if (f.size() != 2)
        return;
    CHECK_EQ(f[0], "XKB_DEFAULT_LAYOUT=tr");
    CHECK_EQ(f[1], "XKB_DEFAULT_VARIANT=f");
    // Giriş ekranına seçim ayrıca iletilir (kendi Appearance'ı dosyayı okuyamaz)
    const auto greeter = keyboardEnvironment("[appearance]\nkeyboardLayout=us\n", true);
    CHECK_EQ(greeter.size(), 2u);
    if (greeter.size() != 2)
        return;
    CHECK_EQ(greeter[0], "XKB_DEFAULT_LAYOUT=us");
    CHECK_EQ(greeter[1], "ANKAROS_KEYBOARD_LAYOUT=us");
    // Seçim yoksa ya da bilinmiyorsa varsayılan (/etc/profile.d) kalır
    CHECK(keyboardEnvironment("[appearance]\ndarkMode=true\n", false).empty());
    CHECK(keyboardEnvironment("[appearance]\nkeyboardLayout=tr;rm\n", true).empty());
    CHECK(keyboardEnvironment("[General]\nkeyboardLayout=us\n", false).empty());
}

#ifdef __linux__
// Denetim soketi komutları sekmeyle ayrılır. Boş alanlar atlanır, bu yüzden boş şifre alan
// sayısını değiştirir (istemciler boş şifreyi göndermeden reddeder, sunucu "yanlış argüman" der).
static void splitsControlFields()
{
    const auto fields = [](const char *line) { return splitFields(line); };
    using Fields = std::vector<std::string>;
    CHECK(fields("status") == Fields{"status"});
    CHECK(fields("login\tali\tgizli") == (Fields{"login", "ali", "gizli"}));
    // Boş satır ve yalnızca ayraçlar komut sayılmaz
    CHECK(fields("").empty());
    CHECK(fields("\t").empty());
    CHECK(fields("\t\t\t").empty());
    // Başta, sonda ve art arda ayraçlar boş alan üretmez
    CHECK(fields("\tstatus") == Fields{"status"});
    CHECK(fields("status\t") == Fields{"status"});
    CHECK(fields("status\t\t") == Fields{"status"});
    CHECK(fields("a\t\tb") == (Fields{"a", "b"}));
    // Boş şifre alanı kaybolur: "unlock" + "" tek alana iner
    CHECK(fields("unlock\t") == Fields{"unlock"});
    CHECK(fields("passwd\t\tyeni") == (Fields{"passwd", "yeni"}));
    // Boşluk ayraç değildir, alan içinde kalır
    CHECK(fields("login\tali\tgizli şifre") == (Fields{"login", "ali", "gizli şifre"}));
    // Tek sekmeli şifre iki alana bölünür (bu yüzden istemciler sekmeyi reddeder)
    CHECK(fields("unlock\tab\tcd") == (Fields{"unlock", "ab", "cd"}));
}

static void hashesPasswords()
{
    const std::string hash = makeHash("gizli şifre");
    CHECK(hash.starts_with("$6$"));
    CHECK(matchesHash("gizli şifre", hash));
    CHECK(!matchesHash("gizli sifre", hash));
    CHECK(!matchesHash("", hash));
    CHECK(makeHash("gizli şifre") != hash); // tuz her seferinde farklı
    CHECK(!matchesHash("", ""));
    // crypt NUL'da keser: "gizli şifre\0x" hash'i "gizli şifre" ile aynı olurdu, reddedilmeli
    CHECK(!matchesHash(std::string("gizli şifre\0x", 15), hash));
    CHECK(!matchesHash("x", "!"));
    CHECK(!matchesHash("x", "*"));
}
#endif

static void choosesDisplayScale()
{
    CHECK(displayScale("1440x900") == 1);
    CHECK(displayScale("1920x1080") == 1);
    CHECK(displayScale("2560x1600") == 2);
    CHECK(displayScale("3680x2300") == 2);
    CHECK(displayScale("") == 1);
    CHECK(displayScale("x900") == 1);
    CHECK(displayScale("abcx900") == 1);
    CHECK(displayScale("99999999999x1") == 1);
    CHECK(displayScale("1280x800", 2) == 2);
    CHECK(displayScale("2880x1800", 1) == 1);
}

static std::string readAll(const std::string &path)
{
    std::ifstream file(path);
    std::ostringstream stream;
    stream << file.rdbuf();
    return stream.str();
}

static void writesSessionLog()
{
    char directory[] = "/tmp/sessionlog-XXXXXX";
    CHECK(mkdtemp(directory) != nullptr);
    const std::string path = std::string(directory) + "/compositor.log";
    {
        SessionLog log(path, 120, [] { return 1.5; });
        log.note("--- ankaros-session: oturum (ankaros) ---");
        // Satırlar parça parça gelebilir, her satırın önünde zaman olur
        log.append("bir", 3);
        const std::string chunk = "inci\niki\n--- ankaros-session: sahte";
        log.append(chunk.data(), chunk.size());
        log.flush();
        CHECK_EQ(readAll(path), "--- ankaros-session: oturum (ankaros) ---\n"
                                "[   1.500] birinci\n"
                                "[   1.500] iki\n"
                                "[   1.500] --- ankaros-session: sahte\n");
        // Sınır aşılınca dosya .1 olur, yeni dosyaya devam edilir
        const std::string line = "uzun bir satir uzun bir satir\n";
        log.append(line.data(), line.size());
        CHECK_EQ(readAll(path), "[   1.500] uzun bir satir uzun bir satir\n");
        CHECK(readAll(path + ".1").find("birinci") != std::string::npos);
        // Satır sonu gelmeyen çok uzun çıktı bellekte birikmez, parça parça yazılır
        const std::string flood(10000, 'x');
        log.append(flood.data(), flood.size());
        CHECK(readAll(path).size() < 5000);
    }
    for (const char *name : {"/compositor.log", "/compositor.log.1"})
        unlink((std::string(directory) + name).c_str());
    rmdir(directory);
}

int main()
{
    writesSessionLog();
    replacesShadowHash();
    readsAndWritesConfig();
    forwardsAppearanceToGreeter();
    forwardsKeyboardLayout();
    choosesDisplayScale();
#ifdef __linux__
    splitsControlFields();
    hashesPasswords();
#endif
    return check::result("session_test");
}
