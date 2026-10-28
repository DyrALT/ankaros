#pragma once

namespace ankaros::log {

// Çekirdeğin açılışından bu yana geçen süre, saniye (CLOCK_BOOTTIME)
double now();

// Satırları ayrıca bir dosyaya da yazar. Dosya açılana kadar yazılanlar bellekte tutulur, açılınca
// dosyaya aktarılır (dosya sistemi bağlanmadan önceki satırlar kaybolmasın).
void openFile(const char *path);

// "[   1.234] mesaj" biçiminde konsola ve dosyaya yazar. printf biçimi.
void write(const char *format, ...) __attribute__((format(printf, 1, 2)));

} // namespace ankaros::log
