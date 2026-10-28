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

int main()
{
    replacesShadowHash();
    readsAndWritesConfig();
    forwardsAppearanceToGreeter();
#ifdef __linux__
    hashesPasswords();
#endif
    return check::result("session_test");
}
