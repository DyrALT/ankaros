#pragma once

#include <string>
#include <string_view>

namespace ankaros {

// /etc/shadow biçimindeki metinde kullanıcının şifre hash'ini değiştirir. Diğer satırlar ve alanlar
// olduğu gibi kalır. Kullanıcı yoksa false döner.
bool replaceShadowHash(std::string &shadow, std::string_view user, std::string_view hash);

// Şifre, crypt(3) hash'iyle eşleşiyor mu? Kilitli ("!", "*") ve boş hash'ler hiçbir şifreyle
// eşleşmez.
bool matchesHash(const std::string &password, const std::string &hash);

// Yeni şifre için SHA-512 crypt hash'i (rastgele tuz). Hata olursa boş döner.
std::string makeHash(const std::string &password);

// /etc/shadow'daki hash ile karşılaştırır (root olarak çalışmalıdır)
bool checkPassword(const std::string &user, const std::string &password);

// /etc/shadow'daki hash'i değiştirir: geçici dosyaya yazıp yerine taşır, izinleri korur.
bool setPassword(const std::string &user, const std::string &password, std::string &error);

} // namespace ankaros
