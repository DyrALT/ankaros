#pragma once

#include "service.h"

#include <string>
#include <vector>

namespace ankaros {

// Bağımlılıkları (after) denetler. Bilinmeyen servislere ve döngü oluşturan bağımlılıklara
// hata yazılır ve bu bağımlılıklar kaldırılır. Servisin kendisi yine başlar: tek bir hatalı
// dosya açılışı durdurmamalı. Servisler ada göre sıralanır, sonuç her açılışta aynıdır.
std::vector<std::string> resolveDependencies(std::vector<ServiceDefinition> &services);

// Bağımlılıklar önce gelecek şekilde sıralı servis adları. resolveDependencies'ten sonra
// çağrılmalıdır. Bağımlılığı aynı olanlar ada göre sıralanır.
std::vector<std::string> startOrder(const std::vector<ServiceDefinition> &services);

} // namespace ankaros
