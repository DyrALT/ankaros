#pragma once

// init, oturum ve ağ servislerinin denetim soketleri. Protokol satır tabanlıdır: istemci tek bir
// komut satırı gönderir ("status", "restart udev"). Sunucu sıfır ya da daha çok veri satırı
// (alanlar sekmeyle ayrılır), ardından "ok" ya da "hata <mesaj>" satırıyla cevap verir ve
// bağlantıyı kapatır.
namespace ankaros {

inline constexpr const char *initSocketPath = "/run/ankaros/init.sock";

} // namespace ankaros
