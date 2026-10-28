#pragma once

#include "service.h"

#include <map>
#include <string>
#include <sys/types.h>
#include <vector>

namespace ankaros {

enum class ServiceState
{
    Waiting,     // bağımlılıkları bekliyor
    Starting,    // oneshot: komutları çalışıyor
    Running,     // simple: çalışıyor
    Done,        // oneshot: başarıyla bitti
    Failed,      // başarısız oldu, yeniden başlatılmayacak
    Stopped,     // durduruldu ya da başarıyla bitti
    RestartWait, // çöktü, yeniden başlatılmayı bekliyor
};

const char *toString(ServiceState state);

struct Service
{
    ServiceDefinition definition;
    ServiceState state = ServiceState::Waiting;
    pid_t pid = 0;
    size_t step = 0;       // oneshot: çalışan komutun sırası
    int restarts = 0;      // art arda kaç kez yeniden başlatıldı
    double startedAt = -1; // ilk başlatılma zamanı (açılıştan beri, sn)
    double readyAt = -1;   // ilk hazır olma zamanı
    double lastStartAt = -1;
    double restartAt = -1; // RestartWait: ne zaman yeniden başlatılacak
    double killAt = -1;    // durdurulurken SIGKILL zamanı
    bool wanted = true;    // false: durdurulması istendi
    bool restartRequested = false;
};

// Servisleri bağımlılık sırasıyla başlatır, izler ve durdurur. Tek iş parçacıklıdır, olaylar
// init'in ana döngüsünden gelir (süreç bitişleri, denetim komutları, zamanlayıcılar).
class Supervisor
{
public:
    // Klasördeki *.service dosyalarını okur. Hatalı dosyalar günlüğe yazılıp atlanır.
    void load(const std::string &directory);
    void add(ServiceDefinition definition);

    bool empty() const { return m_services.empty(); }

    // Bağımlılıkları hazır olan servisleri başlatır, zamanı gelen yeniden başlatmaları yapar.
    void update();
    // Ana döngünün en fazla ne kadar bekleyebileceği (sn). Bekleyen zamanlayıcı yoksa -1.
    double timeout() const;
    // waitpid'den gelen süreç bitişi. pid bir servise ait değilse false (sahipsiz süreç).
    bool childExited(pid_t pid, int status);

    bool start(const std::string &name, std::string &error);
    bool stop(const std::string &name, std::string &error);
    bool restart(const std::string &name, std::string &error);
    // Kapatma: servisleri başlatılma sırasının tersiyle durdurur, en fazla timeout saniye bekler.
    void stopAll(double timeout);

    void mark(const std::string &name);

    // Denetim soketi için sekmeyle ayrılmış satırlar (bkz. control.cpp)
    std::vector<std::string> statusLines() const;
    std::vector<std::string> bootLines() const;

private:
    Service *find(const std::string &name);
    bool dependenciesSettled(const Service &service) const;
    void spawn(Service &service);
    void finish(Service &service, ServiceState state);
    void terminate(Service &service);

    std::vector<Service> m_services; // başlatma sırasıyla
    std::vector<std::pair<std::string, double>> m_marks;
};

} // namespace ankaros
