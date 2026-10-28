#include "service.h"

#include "command.h"

#include <algorithm>
#include <array>
#include <cctype>

namespace ankaros {

namespace {

std::string at(int line, const std::string &message)
{
    return "satır " + std::to_string(line) + ": " + message;
}

} // namespace

bool isValidServiceName(std::string_view name)
{
    if (name.empty() || name.size() > 64)
        return false;
    return std::all_of(name.begin(), name.end(), [](char c) {
        return std::islower(static_cast<unsigned char>(c)) ||
               std::isdigit(static_cast<unsigned char>(c)) || c == '-' || c == '_' || c == '.';
    });
}

bool parseService(std::string_view name, const Config &config, ServiceDefinition &service,
                  std::vector<std::string> &errors)
{
    const size_t errorCount = errors.size();
    for (const ConfigError &error : config.errors)
        errors.push_back(at(error.line, error.message));

    service = {};
    service.name = std::string(name);
    if (!isValidServiceName(name))
        errors.push_back("geçersiz servis adı: " + service.name);

    bool restartGiven = false;
    for (const ConfigEntry &entry : config.entries) {
        const std::string &value = entry.value;
        if (entry.key == "description") {
            service.description = value;
        } else if (entry.key == "type") {
            if (value == "simple")
                service.type = ServiceType::Simple;
            else if (value == "oneshot")
                service.type = ServiceType::Oneshot;
            else
                errors.push_back(at(entry.line, "type simple ya da oneshot olmalı: " + value));
        } else if (entry.key == "restart") {
            restartGiven = true;
            if (value == "no")
                service.restart = RestartPolicy::No;
            else if (value == "on-failure")
                service.restart = RestartPolicy::OnFailure;
            else if (value == "always")
                service.restart = RestartPolicy::Always;
            else
                errors.push_back(
                    at(entry.line, "restart no, on-failure ya da always olmalı: " + value));
        } else if (entry.key == "exec") {
            std::vector<std::string> arguments;
            std::string error;
            if (!splitCommand(value, arguments, error))
                errors.push_back(at(entry.line, error));
            else if (arguments.empty())
                errors.push_back(at(entry.line, "exec boş"));
            else
                service.commands.push_back(std::move(arguments));
        } else if (entry.key == "after") {
            for (size_t start = 0; start < value.size();) {
                const size_t end = std::min(value.find(' ', start), value.size());
                const std::string dependency = value.substr(start, end - start);
                start = end + 1;
                if (dependency.empty())
                    continue;
                if (!isValidServiceName(dependency))
                    errors.push_back(at(entry.line, "geçersiz servis adı: " + dependency));
                else if (dependency == service.name)
                    errors.push_back(at(entry.line, "servis kendine bağımlı olamaz"));
                else if (std::find(service.after.begin(), service.after.end(), dependency) ==
                         service.after.end())
                    service.after.push_back(dependency);
            }
        } else if (entry.key == "user") {
            service.user = value;
        } else if (entry.key == "environment") {
            const size_t equals = value.find('=');
            if (equals == 0 || equals == std::string::npos)
                errors.push_back(at(entry.line, "environment AD=değer biçiminde olmalı"));
            else
                service.environment.push_back(value);
        } else {
            errors.push_back(at(entry.line, "bilinmeyen ad: " + entry.key));
        }
    }

    if (service.commands.empty())
        errors.push_back("exec yok");
    if (service.type == ServiceType::Simple && service.commands.size() > 1)
        errors.push_back("simple serviste tek exec olabilir, birden çok komut için type = oneshot");
    if (service.type == ServiceType::Oneshot) {
        if (restartGiven && service.restart != RestartPolicy::No)
            errors.push_back("oneshot servis yeniden başlatılamaz (restart = no)");
        service.restart = RestartPolicy::No;
    }

    return errors.size() == errorCount;
}

const char *toString(ServiceType type)
{
    return type == ServiceType::Simple ? "simple" : "oneshot";
}

const char *toString(RestartPolicy policy)
{
    static constexpr std::array names = {"no", "on-failure", "always"};
    return names[static_cast<size_t>(policy)];
}

} // namespace ankaros
