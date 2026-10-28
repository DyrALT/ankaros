#include "netconfig.h"

#include "config.h"

#include <arpa/inet.h>
#include <cstdio>
#include <cstdlib>

namespace ankaros {

std::vector<std::string> splitWords(std::string_view text)
{
    std::vector<std::string> words;
    size_t start = 0;
    while (start < text.size()) {
        while (start < text.size() && (text[start] == ' ' || text[start] == '\t'))
            ++start;
        size_t end = start;
        while (end < text.size() && text[end] != ' ' && text[end] != '\t')
            ++end;
        if (end > start)
            words.emplace_back(text.substr(start, end - start));
        start = end;
    }
    return words;
}

NetworkConfig parseNetworkConfig(const std::string &text)
{
    const Config config = parseConfig(text);
    NetworkConfig network;
    network.mode = config.value("mode", "dhcp");
    network.address = config.value("address");
    network.gateway = config.value("gateway");
    network.dns = splitWords(config.value("dns"));
    return network;
}

std::string formatNetworkConfig(const NetworkConfig &config)
{
    std::string text =
        "# ankaros ağ ayarları (docs/ag.md). Ayarlar › Ağ'dan da değiştirilir.\n"
        "# mode: dhcp (adres ağdan otomatik alınır) ya da static (aşağıdaki değerler)\n"
        "mode = " +
        config.mode + "\n";
    if (config.mode == "static") {
        std::string dns;
        for (const std::string &server : config.dns)
            dns += (dns.empty() ? "" : " ") + server;
        text += "address = " + config.address + "\n";
        text += "gateway = " + config.gateway + "\n";
        text += "dns = " + dns + "\n";
    }
    return text;
}

bool isIPv4(std::string_view text)
{
    in_addr address{};
    const std::string copy(text);
    return !copy.empty() && inet_pton(AF_INET, copy.c_str(), &address) == 1;
}

bool parseCidr(std::string_view text, std::string &address, int &prefix)
{
    const size_t slash = text.find('/');
    if (slash == std::string_view::npos)
        return false;
    address = std::string(text.substr(0, slash));
    const std::string number(text.substr(slash + 1));
    if (number.empty() || number.size() > 2 ||
        number.find_first_not_of("0123456789") != std::string::npos)
        return false;
    prefix = std::atoi(number.c_str());
    return isIPv4(address) && prefix >= 1 && prefix <= 32;
}

std::string validateNetworkConfig(const NetworkConfig &config)
{
    if (config.mode == "dhcp")
        return {};
    if (config.mode != "static")
        return "Bilinmeyen yöntem: " + config.mode;
    std::string address;
    int prefix = 0;
    if (!parseCidr(config.address, address, prefix))
        return "Adres 192.168.1.50/24 biçiminde olmalı";
    if (!config.gateway.empty() && !isIPv4(config.gateway))
        return "Ağ geçidi geçerli bir IPv4 adresi değil";
    for (const std::string &server : config.dns) {
        if (!isIPv4(server))
            return "DNS sunucusu geçerli bir IPv4 adresi değil: " + server;
    }
    return {};
}

std::string defaultGateway(std::string_view procNetRoute, std::string_view interface)
{
    // Biçim: Iface Destination Gateway Flags ... (adresler küçük sonlu onaltılık)
    size_t start = 0;
    while (start < procNetRoute.size()) {
        const size_t end = std::min(procNetRoute.find('\n', start), procNetRoute.size());
        const std::vector<std::string> fields = splitWords(procNetRoute.substr(start, end - start));
        start = end + 1;
        if (fields.size() < 3 || fields[0] != interface || fields[1] != "00000000")
            continue;
        const unsigned long gateway = std::strtoul(fields[2].c_str(), nullptr, 16);
        if (gateway == 0)
            continue;
        char text[16];
        std::snprintf(text, sizeof text, "%lu.%lu.%lu.%lu", gateway & 0xff, (gateway >> 8) & 0xff,
                      (gateway >> 16) & 0xff, (gateway >> 24) & 0xff);
        return text;
    }
    return {};
}

std::vector<std::string> nameservers(std::string_view resolvConf)
{
    std::vector<std::string> servers;
    size_t start = 0;
    while (start < resolvConf.size()) {
        const size_t end = std::min(resolvConf.find('\n', start), resolvConf.size());
        const std::vector<std::string> fields = splitWords(resolvConf.substr(start, end - start));
        start = end + 1;
        if (fields.size() >= 2 && fields[0] == "nameserver")
            servers.push_back(fields[1]);
    }
    return servers;
}

std::string formatResolvConf(const std::vector<std::string> &dns)
{
    std::string text = "# ankaros-network tarafından yazıldı\n";
    for (const std::string &server : dns)
        text += "nameserver " + server + "\n";
    return text;
}

} // namespace ankaros
