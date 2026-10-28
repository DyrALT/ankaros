#pragma once

#include "config.h"

#include <string>
#include <string_view>
#include <vector>

namespace ankaros {

enum class ServiceType
{
    Simple,  // sürekli çalışır, başlatıldığında hazır sayılır
    Oneshot, // komutları sırayla çalışıp biter, başarıyla bittiğinde hazır sayılır
};

enum class RestartPolicy
{
    No,
    OnFailure, // sıfırdan farklı çıkış kodu ya da sinyalle biterse
    Always,
};

// /etc/ankaros/services/<ad>.service dosyasının içeriği. Biçim: docs/init.md
struct ServiceDefinition
{
    std::string name;
    std::string description;
    ServiceType type = ServiceType::Simple;
    RestartPolicy restart = RestartPolicy::OnFailure;
    std::vector<std::vector<std::string>> commands;
    std::vector<std::string> after;
    std::string user;
    std::vector<std::string> environment; // "AD=değer"
};

bool isValidServiceName(std::string_view name);

// Servis dosyasını doğrular. Hatalar "satır N: ..." biçiminde errors'a eklenir, hata varsa
// false döner ve servis başlatılmamalıdır.
bool parseService(std::string_view name, const Config &config, ServiceDefinition &service,
                  std::vector<std::string> &errors);

const char *toString(ServiceType type);
const char *toString(RestartPolicy policy);

} // namespace ankaros
