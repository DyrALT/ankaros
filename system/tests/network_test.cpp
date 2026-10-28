#include "check.h"
#include "netconfig.h"

using namespace ankaros;

static void parsesConfig()
{
    NetworkConfig config = parseNetworkConfig("mode = static\naddress = 192.168.1.50/24\n"
                                              "gateway = 192.168.1.1\ndns = 1.1.1.1  9.9.9.9\n");
    CHECK_EQ(config.mode, "static");
    CHECK_EQ(config.address, "192.168.1.50/24");
    CHECK_EQ(config.dns.size(), 2u);
    CHECK_EQ(config.dns[1], "9.9.9.9");
    CHECK(validateNetworkConfig(config).empty());
    const NetworkConfig again = parseNetworkConfig(formatNetworkConfig(config));
    CHECK_EQ(again.address, config.address);
    CHECK_EQ(again.gateway, config.gateway);
    CHECK_EQ(again.dns.size(), 2u);

    CHECK_EQ(parseNetworkConfig("").mode, "dhcp");
    CHECK_EQ(parseNetworkConfig(formatNetworkConfig({})).mode, "dhcp");
}

static void validates()
{
    std::string address;
    int prefix = 0;
    CHECK(parseCidr("10.0.2.15/24", address, prefix));
    CHECK_EQ(address, "10.0.2.15");
    CHECK_EQ(prefix, 24);
    CHECK(!parseCidr("10.0.2.15", address, prefix));
    CHECK(!parseCidr("10.0.2.15/33", address, prefix));
    CHECK(!parseCidr("10.0.2.15/", address, prefix));
    CHECK(!parseCidr("10.0.2/24", address, prefix));
    CHECK(!parseCidr("256.1.1.1/24", address, prefix));
    CHECK(!parseCidr("1.1.1.1/2x", address, prefix));
    CHECK(isIPv4("1.1.1.1"));
    CHECK(!isIPv4("1.1.1"));
    CHECK(!isIPv4(""));

    NetworkConfig config;
    config.mode = "static";
    config.address = "192.168.1.50";
    CHECK(!validateNetworkConfig(config).empty());
    config.address = "192.168.1.50/24";
    config.gateway = "x";
    CHECK(!validateNetworkConfig(config).empty());
    config.gateway = "";
    config.dns = {"1.1.1.1", "yok"};
    CHECK(!validateNetworkConfig(config).empty());
    config.mode = "pppoe";
    CHECK(!validateNetworkConfig(config).empty());
}

static void readsKernelState()
{
    const char *route = "Iface\tDestination\tGateway \tFlags\tRefCnt\tUse\tMetric\tMask\n"
                        "eth0\t00000000\t0202000A\t0003\t0\t0\t0\t00000000\n"
                        "eth0\t0002000A\t00000000\t0001\t0\t0\t0\t00FFFFFF\n";
    CHECK_EQ(defaultGateway(route, "eth0"), "10.0.2.2");
    CHECK_EQ(defaultGateway(route, "eth1"), "");
    CHECK_EQ(defaultGateway("", "eth0"), "");

    const auto servers =
        nameservers("# yorum\nnameserver 10.0.2.3 # eth0\nsearch lan\nnameserver 1.1.1.1\n");
    CHECK_EQ(servers.size(), 2u);
    CHECK_EQ(servers[0], "10.0.2.3");
    CHECK_EQ(nameservers(formatResolvConf({"1.1.1.1", "9.9.9.9"})).size(), 2u);
}

int main()
{
    parsesConfig();
    validates();
    readsKernelState();
    return check::result("network_test");
}
