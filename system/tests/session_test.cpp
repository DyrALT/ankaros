#include "accounts.h"
#include "check.h"
#include "sessionconfig.h"

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
    CHECK_EQ(f[0], "XKB_DEFAULT_LAYOUT=tr");
    CHECK_EQ(f[1], "XKB_DEFAULT_VARIANT=f");
    // Giriş ekranına seçim ayrıca iletilir (kendi Appearance'ı dosyayı okuyamaz)
    const auto greeter = keyboardEnvironment("[appearance]\nkeyboardLayout=us\n", true);
    CHECK_EQ(greeter.size(), 2u);
    CHECK_EQ(greeter[0], "XKB_DEFAULT_LAYOUT=us");
    CHECK_EQ(greeter[1], "ANKAROS_KEYBOARD_LAYOUT=us");
    // Seçim yoksa ya da bilinmiyorsa varsayılan (/etc/profile.d) kalır
    CHECK(keyboardEnvironment("[appearance]\ndarkMode=true\n", false).empty());
    CHECK(keyboardEnvironment("[appearance]\nkeyboardLayout=tr;rm\n", true).empty());
    CHECK(keyboardEnvironment("[General]\nkeyboardLayout=us\n", false).empty());
}

#ifdef __linux__
static void hashesPasswords()
{
    const std::string hash = makeHash("gizli şifre");
    CHECK(hash.starts_with("$6$"));
    CHECK(matchesHash("gizli şifre", hash));
    CHECK(!matchesHash("gizli sifre", hash));
    CHECK(!matchesHash("", hash));
    CHECK(makeHash("gizli şifre") != hash); // tuz her seferinde farklı
    CHECK(!matchesHash("", ""));
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

int main()
{
    replacesShadowHash();
    readsAndWritesConfig();
    forwardsAppearanceToGreeter();
    forwardsKeyboardLayout();
    choosesDisplayScale();
#ifdef __linux__
    hashesPasswords();
#endif
    return check::result("session_test");
}
