#include "check.h"
#include "layer.h"

using namespace ankaros;

static void validatesLayerNames()
{
    CHECK(validLayerName("debian"));
    CHECK(validLayerName("fedora-41"));
    CHECK(!validLayerName(""));
    CHECK(!validLayerName("-x"));
    CHECK(!validLayerName("../etc"));
    CHECK(!validLayerName("deb/ian"));
    CHECK(!validLayerName("Debian"));
    CHECK(!validLayerName(std::string(33, 'a')));
    CHECK_EQ(layerRoot("debian"), "/var/lib/ankaros/katmanlar/debian/kok");
}

static void sharesPaths()
{
    // Root (apt): yalnızca aygıtlar, /proc, /sys, /tmp ve ad çözümü
    const auto system = sharedPaths("", "");
    CHECK_EQ(system.size(), 5u);
    CHECK(system[4].file);

    // Kullanıcı: ev klasörü ve Wayland soketinin klasörü de
    const auto user = sharedPaths("/home/ankaros", "/run/user/1000");
    CHECK_EQ(user.size(), 7u);
    CHECK_EQ(user[5].target, "/home/ankaros");
    CHECK_EQ(user[6].target, "/run/user/1000");

    // Katmanın tmpfs'leri dışındaki yerlere bağlanmaz
    CHECK_EQ(sharedPaths("/root", "/var/run").size(), 5u);
}

int main()
{
    validatesLayerNames();
    sharesPaths();
    return check::result("katman_test");
}
