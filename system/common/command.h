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

} // namespace ankaros
