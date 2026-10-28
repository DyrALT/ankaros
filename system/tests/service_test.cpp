#include "check.h"
#include "graph.h"
#include "service.h"

#include <algorithm>

using namespace ankaros;

static ServiceDefinition parse(const char *name, const char *text, std::vector<std::string> *errors)
{
    ServiceDefinition service;
    std::vector<std::string> ignored;
    parseService(name, parseConfig(text), service, errors ? *errors : ignored);
    return service;
}

static void parsesService()
{
    std::vector<std::string> errors;
    const ServiceDefinition udev = parse("udev",
                                         "description = Aygıt yöneticisi\n"
                                         "type = oneshot\n"
                                         "exec = /sbin/udevd --daemon\n"
                                         "exec = /sbin/udevadm settle\n"
                                         "after = seedrng  syslogd seedrng\n"
                                         "environment = A=1\n",
                                         &errors);
    CHECK(errors.empty());
    CHECK(udev.type == ServiceType::Oneshot);
    CHECK(udev.restart == RestartPolicy::No);
    CHECK_EQ(udev.commands.size(), 2u);
    CHECK_EQ(udev.commands[1][0], "/sbin/udevadm");
    CHECK_EQ(udev.after.size(), 2u); // tekrarlanan bağımlılık bir kez
    CHECK_EQ(udev.environment[0], "A=1");

    const ServiceDefinition getty = parse("getty", "exec = getty\nrestart = always\n", &errors);
    CHECK(errors.empty());
    CHECK(getty.type == ServiceType::Simple);
    CHECK(getty.restart == RestartPolicy::Always);

    const ServiceDefinition simple = parse("a", "exec = a\n", &errors);
    CHECK(simple.restart == RestartPolicy::OnFailure); // simple için varsayılan
}

static void rejectsInvalid()
{
    const auto errorsFor = [](const char *name, const char *text) {
        std::vector<std::string> errors;
        parse(name, text, &errors);
        return errors.size();
    };
    CHECK_EQ(errorsFor("a", "description = exec yok\n"), 1u);
    CHECK_EQ(errorsFor("a", "exec = a\nexec = b\n"), 1u); // simple'da iki exec
    CHECK_EQ(errorsFor("a", "type = oneshot\nexec = a\nrestart = always\n"), 1u);
    CHECK_EQ(errorsFor("a", "exec = a\ntype = daemon\n"), 1u);
    CHECK_EQ(errorsFor("a", "exec = a\nresart = no\n"), 1u); // yazım hatası
    CHECK_EQ(errorsFor("a", "exec = a\nafter = a\n"), 1u);   // kendine bağımlı
    CHECK_EQ(errorsFor("a", "exec = a\nenvironment = yok\n"), 1u);
    CHECK_EQ(errorsFor("Büyük", "exec = a\n"), 1u);
    CHECK_EQ(errorsFor("a", "exec = 'kapanmıyor\n"), 2u); // tırnak ve exec yok
}

static ServiceDefinition service(const char *name, std::vector<std::string> after)
{
    ServiceDefinition definition;
    definition.name = name;
    definition.after = std::move(after);
    definition.commands = {{"true"}};
    return definition;
}

static size_t position(const std::vector<std::string> &order, const std::string &name)
{
    return static_cast<size_t>(std::find(order.begin(), order.end(), name) - order.begin());
}

static void ordersByDependencies()
{
    std::vector<ServiceDefinition> services = {
        service("compositor", {"udev"}), service("udev", {"seedrng"}),
        service("seedrng", {}),          service("getty", {}),
        service("network", {"udev"}),
    };
    CHECK(resolveDependencies(services).empty());
    const std::vector<std::string> order = startOrder(services);
    CHECK_EQ(order.size(), 5u);
    CHECK(position(order, "seedrng") < position(order, "udev"));
    CHECK(position(order, "udev") < position(order, "compositor"));
    CHECK(position(order, "udev") < position(order, "network"));
    // Bağımlılığı olmayanlar ada göre önce gelir
    CHECK_EQ(order[0], "getty");
    CHECK_EQ(order[1], "seedrng");
}

static void dropsUnknownAndCyclicDependencies()
{
    std::vector<ServiceDefinition> services = {
        service("a", {"b"}),
        service("b", {"c"}),
        service("c", {"a", "yok"}),
        service("d", {"a"}),
    };
    const std::vector<std::string> errors = resolveDependencies(services);
    CHECK_EQ(errors.size(), 2u); // bilinmeyen "yok" ve a → b → c → a döngüsü
    const std::vector<std::string> order = startOrder(services);
    CHECK_EQ(order.size(), 4u); // hepsi yine başlar
    CHECK(position(order, "a") < position(order, "d"));
}

int main()
{
    parsesService();
    rejectsInvalid();
    ordersByDependencies();
    dropsUnknownAndCyclicDependencies();
    return check::result("service_test");
}
