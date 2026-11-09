#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace ankaros {

// Komut satırını argümanlara böler. Tek tırnak içi olduğu gibi alınır. Çift tırnak içinde ve
// tırnak dışında ters bölü bir sonraki karakteri kaçırır. Kabuk özellikleri (değişkenler,
// yönlendirme, boru) yoktur, gerekirse komut "sh -c '...'" olarak yazılır.
// Kapanmayan tırnakta false döner ve error doldurulur.
bool splitCommand(std::string_view command, std::vector<std::string> &arguments,
                  std::string &error);

// Günlüğe ve konsola yazılacak metindeki kontrol karakterlerini '?' ile değiştirir: C0 baytları
// (0x00-0x1f, 0x7f) ve UTF-8 olarak kodlanmış C1 karakterleri (U+0080-U+009F). Türkçe harfler gibi
// diğer UTF-8 dizilerine dokunmaz.
std::string stripControlCharacters(std::string_view text);

} // namespace ankaros
