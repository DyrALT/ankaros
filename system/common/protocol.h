#pragma once

// init, oturum ve ağ servislerinin denetim soketleri. Protokol satır tabanlıdır: istemci tek bir
// komut satırı gönderir ("status", "restart udev"). Sunucu sıfır ya da daha çok veri satırı
// (alanlar sekmeyle ayrılır), ardından "ok" ya da "hata <mesaj>" satırıyla cevap verir ve
// bağlantıyı kapatır.
#include <string>
#include <vector>

namespace ankaros {

inline constexpr const char *initSocketPath = "/run/ankaros/init.sock";
inline constexpr const char *sessionSocketPath = "/run/ankaros/session.sock";

// Komutu gönderir, cevaptaki veri satırlarını lines'a koyar. Hata mesajını döner, başarılıysa boş.
std::string request(const char *socketPath, const std::vector<std::string> &command,
                    std::vector<std::string> &lines);

} // namespace ankaros
