#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace ankaros {

// /etc/ankaros/network.conf. Biçim: docs/ag.md
//   mode = dhcp | static
//   address = 192.168.1.50/24   (static)
//   gateway = 192.168.1.1       (static, isteğe bağlı)
//   dns = 1.1.1.1 9.9.9.9       (static, isteğe bağlı)
struct NetworkConfig
{
    std::string mode = "dhcp";
    std::string address;
    std::string gateway;
    std::vector<std::string> dns;
};

NetworkConfig parseNetworkConfig(const std::string &text);
std::string formatNetworkConfig(const NetworkConfig &config);

bool isIPv4(std::string_view text);
// "192.168.1.50/24" -> adres ve önek uzunluğu
bool parseCidr(std::string_view text, std::string &address, int &prefix);
// Ayarın geçerli olup olmadığı. Geçersizse ekranda gösterilecek hata mesajı, geçerliyse boş.
std::string validateNetworkConfig(const NetworkConfig &config);

// /proc/net/route içeriğinden arayüzün varsayılan ağ geçidi, yoksa boş
std::string defaultGateway(std::string_view procNetRoute, std::string_view interface);
// resolv.conf içindeki nameserver satırları
std::vector<std::string> nameservers(std::string_view resolvConf);
std::string formatResolvConf(const std::vector<std::string> &dns);

std::vector<std::string> splitWords(std::string_view text);

} // namespace ankaros
