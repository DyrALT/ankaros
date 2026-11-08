#pragma once

#include <cstddef>
#include <functional>
#include <string>

namespace ankaros {

// Masaüstünün (compositor ve açtığı uygulamalar) çıktısını günlük dosyasına yazar. Çıktı oturum
// yöneticisine bir borudan gelir, uygulamalar dosyaya doğrudan yazamaz:
//   - dosya en fazla maxSize bayta büyür, sonra PATH.1 olur (en fazla iki dosya)
//   - her satırın başına oturum yöneticisi zaman yazar ("[  12.345] "). Uygulamanın yazdığı her şey
//     bunun arkasında kalır, oturum yöneticisinin kendi satırları ("--- ankaros-session: ...")
//     taklit edilemez.
// Dosya açılamazsa çıktı atılır, boru yine boşaltılır (uygulamalar yazarken takılmasın).
class SessionLog
{
public:
    using Clock = std::function<double()>;

    SessionLog(std::string path, size_t maxSize, Clock clock);
    ~SessionLog();
    SessionLog(const SessionLog &) = delete;
    SessionLog &operator=(const SessionLog &) = delete;

    // Borudan gelen ham çıktı. Yarım kalan satır sonraki parçayla birleştirilir.
    void append(const char *data, size_t size);
    // Yarım kalan satırı yazar (boru kapanınca)
    void flush();
    // Oturum yöneticisinin kendi satırı (önünde zaman yok, "---" ile başlar)
    void note(const std::string &line);

private:
    void writeLine(const std::string &line, bool stamped);
    void open();
    void rotate();

    std::string m_path;
    size_t m_maxSize;
    Clock m_clock;
    int m_fd = -1;
    size_t m_size = 0;
    std::string m_partial;
};

} // namespace ankaros
