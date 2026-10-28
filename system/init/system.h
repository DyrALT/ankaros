#pragma once

namespace ankaros {

enum class PowerAction
{
    Poweroff,
    Reboot,
    Halt,
};

const char *toString(PowerAction action);

// Servislerden önce: sanal dosya sistemleri, kök dosya sisteminin yazılabilir yapılması, makine
// adı, /run/ankaros. Hatalar günlüğe yazılır, açılış devam eder.
void prepareSystem();

// Servisler durdurulduktan sonra: kalan bütün süreçler sonlandırılır, diskler senkronlanır, kök
// dosya sistemi salt okunur yapılır ve makine kapatılır ya da yeniden başlatılır.
[[noreturn]] void finishShutdown(PowerAction action);

// waitpid(-1) ile bütün çocukları toplar, bitenleri callback'e bildirir. Bekleyen çocuk kalmadıysa
// false döner.
template <typename Callback> bool reapChildren(Callback callback);

} // namespace ankaros

#include <sys/wait.h>

template <typename Callback> bool ankaros::reapChildren(Callback callback)
{
    for (;;) {
        int status = 0;
        const pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid > 0) {
            callback(pid, status);
            continue;
        }
        return pid == 0; // 0: çocuk var ama bitmedi, -1 (ECHILD): çocuk kalmadı
    }
}
