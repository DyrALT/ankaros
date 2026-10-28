#include "check.h"
#include "command.h"
#include "config.h"

using namespace ankaros;

static void parsesEntries()
{
    const Config config = parseConfig("# yorum\n"
                                      "\n"
                                      "description = Aygıt yöneticisi\n"
                                      "exec=/sbin/udevd --daemon   # satır sonu yorumu\n"
                                      "  exec = /sbin/udevadm settle\n"
                                      "url = http://a#b\n");
    CHECK(config.errors.empty());
    CHECK_EQ(config.entries.size(), 4u);
    CHECK_EQ(config.value("description"), "Aygıt yöneticisi");
    CHECK_EQ(config.values("exec").size(), 2u);
    CHECK_EQ(config.values("exec")[0], "/sbin/udevd --daemon");
    CHECK_EQ(config.entries[2].line, 5);
    CHECK_EQ(config.value("url"), "http://a#b"); // önünde boşluk olmayan # yorum değil
    CHECK_EQ(config.value("yok", "varsayılan"), "varsayılan");
    CHECK(!config.has("yok"));
}

static void reportsErrors()
{
    const Config config = parseConfig("eşittir yok\n= değer\nBüyük = x\ntamam = 1\n");
    CHECK_EQ(config.errors.size(), 3u);
    CHECK_EQ(config.errors[0].line, 1);
    CHECK_EQ(config.errors[1].line, 2);
    CHECK_EQ(config.errors[2].line, 3);
    CHECK_EQ(config.value("tamam"), "1");
}

static void lastValueWins()
{
    const Config config = parseConfig("mode = dhcp\nmode = static\n");
    CHECK_EQ(config.value("mode"), "static");
}

static void splitsCommands()
{
    std::vector<std::string> arguments;
    std::string error;
    CHECK(splitCommand("  /sbin/getty -L  console 0 vt100 ", arguments, error));
    CHECK_EQ(arguments.size(), 5u);
    CHECK_EQ(arguments[2], "console");

    CHECK(splitCommand("/bin/sh -c '. /etc/profile; exec ankaros-compositor'", arguments, error));
    CHECK_EQ(arguments.size(), 3u);
    CHECK_EQ(arguments[2], ". /etc/profile; exec ankaros-compositor");

    CHECK(splitCommand(R"(echo "a \"b\" c" d\ e '' "")", arguments, error));
    CHECK_EQ(arguments.size(), 5u);
    CHECK_EQ(arguments[1], "a \"b\" c");
    CHECK_EQ(arguments[2], "d e");
    CHECK_EQ(arguments[3], "");
    CHECK_EQ(arguments[4], "");

    CHECK(splitCommand("'tek\\tırnak'", arguments, error));
    CHECK_EQ(arguments[0], "tek\\tırnak");

    CHECK(!splitCommand("echo 'kapanmıyor", arguments, error));
    CHECK(!error.empty());

    CHECK(splitCommand("   ", arguments, error));
    CHECK(arguments.empty());
}

int main()
{
    parsesEntries();
    reportsErrors();
    lastValueWins();
    splitsCommands();
    return check::result("config_test");
}
