#include "graph.h"

#include <algorithm>
#include <map>

namespace ankaros {

namespace {

enum class Mark
{
    None,
    Visiting,
    Done,
};

struct CycleBreaker
{
    std::map<std::string, ServiceDefinition *> byName;
    std::map<std::string, Mark> marks;
    std::vector<std::string> path;
    std::vector<std::string> errors;

    void visit(ServiceDefinition &service)
    {
        marks[service.name] = Mark::Visiting;
        path.push_back(service.name);
        for (auto it = service.after.begin(); it != service.after.end();) {
            ServiceDefinition *dependency = byName.at(*it);
            const Mark mark = marks[*it];
            if (mark == Mark::Visiting) {
                // Döngüyü yazdır ve bu kenarı kaldır
                std::string cycle;
                const auto start = std::find(path.begin(), path.end(), *it);
                for (auto step = start; step != path.end(); ++step)
                    cycle += *step + " → ";
                errors.push_back(service.name + ": döngüsel bağımlılık (" + cycle + *it + "), " +
                                 *it + " bağımlılığı yok sayıldı");
                it = service.after.erase(it);
                continue;
            }
            if (mark == Mark::None)
                visit(*dependency);
            ++it;
        }
        path.pop_back();
        marks[service.name] = Mark::Done;
    }
};

} // namespace

std::vector<std::string> resolveDependencies(std::vector<ServiceDefinition> &services)
{
    std::sort(
        services.begin(), services.end(),
        [](const ServiceDefinition &a, const ServiceDefinition &b) { return a.name < b.name; });

    CycleBreaker breaker;
    for (ServiceDefinition &service : services)
        breaker.byName[service.name] = &service;

    for (ServiceDefinition &service : services) {
        std::erase_if(service.after, [&](const std::string &dependency) {
            if (breaker.byName.contains(dependency))
                return false;
            breaker.errors.push_back(service.name + ": bilinmeyen bağımlılık " + dependency +
                                     " yok sayıldı");
            return true;
        });
    }

    for (ServiceDefinition &service : services) {
        if (breaker.marks[service.name] == Mark::None)
            breaker.visit(service);
    }
    return breaker.errors;
}

std::vector<std::string> startOrder(const std::vector<ServiceDefinition> &services)
{
    // Kahn algoritması: her turda bağımlılıkları tamamlanmış servisler ada göre eklenir.
    std::vector<std::string> order;
    std::vector<const ServiceDefinition *> remaining;
    for (const ServiceDefinition &service : services)
        remaining.push_back(&service);
    std::sort(remaining.begin(), remaining.end(),
              [](const auto *a, const auto *b) { return a->name < b->name; });

    while (!remaining.empty()) {
        std::vector<const ServiceDefinition *> ready;
        for (const ServiceDefinition *service : remaining) {
            const bool satisfied =
                std::all_of(service->after.begin(), service->after.end(), [&](const auto &d) {
                    return std::find(order.begin(), order.end(), d) != order.end();
                });
            if (satisfied)
                ready.push_back(service);
        }
        if (ready.empty())
            break; // resolveDependencies çağrılmamış, döngü var
        for (const ServiceDefinition *service : ready) {
            order.push_back(service->name);
            std::erase(remaining, service);
        }
    }
    return order;
}

} // namespace ankaros
