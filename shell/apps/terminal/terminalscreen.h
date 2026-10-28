#pragma once

#include <QColor>
#include <QString>

#include <deque>
#include <functional>
#include <vector>
#include <vterm.h>

// Terminalin ekran durumu: libvterm'i sarar (karar 0014). Kabuktan gelen baytları yorumlar, görünen
// satırları ve kaydırma geçmişini tutar, tuşları kabuğa gidecek baytlara çevirir. Çizim ve pty
// TerminalView'dadır, bu sınıf Qt Quick'e bağlı değildir (birim testleri bunu kullanır).
class TerminalScreen
{
public:
    // Bir hücre. Geniş karakterin (ör. 漢) ikinci hücresinin metni boş, genişliği 0'dır.
    struct Cell
    {
        QString text;
        // Geçersizse varsayılan ön ya da arka plan rengi kullanılır
        QColor foreground;
        QColor background;
        // 0-15 arası ANSI rengi, palet temaya göre değiştiği için ayrı tutulur (-1: yok)
        int foregroundIndex = -1;
        int backgroundIndex = -1;
        bool bold = false;
        bool italic = false;
        bool underline = false;
        bool reverse = false;
        int width = 1;
    };
    using Line = std::vector<Cell>;

    // Satırların mutlak numarası: 0 kaydırma geçmişinin en eski satırı, geçmiş doldukça kayar.
    struct Position
    {
        int line = 0;
        int column = 0;
        bool operator<(const Position &other) const
        {
            return line != other.line ? line < other.line : column < other.column;
        }
        bool operator==(const Position &other) const = default;
    };

    TerminalScreen(int rows, int columns, int scrollbackLimit = 5000);
    ~TerminalScreen();
    TerminalScreen(const TerminalScreen &) = delete;
    TerminalScreen &operator=(const TerminalScreen &) = delete;

    // Kabuğun çıktısı
    void write(const char *data, size_t size);
    void write(const QByteArray &data) { write(data.constData(), size_t(data.size())); }
    void resize(int rows, int columns);

    // Klavye. text boş değilse yazılan metin gönderilir, değilse özel tuş (Qt::Key).
    // Ctrl+harf kontrol karakterine çevrilir.
    void key(int qtKey, Qt::KeyboardModifiers modifiers, const QString &text);
    // Yapıştırma: kabuk istediyse "bracketed paste" dizileriyle sarılır
    void paste(const QString &text);

    int rows() const { return m_rows; }
    int columns() const { return m_columns; }
    int scrollbackSize() const { return int(m_scrollback.size()); }
    // Geçmiş sınırına gelince en eski satırlar atılır. Seçim gibi mutlak konumlar bu kadar kayar.
    int droppedLines() const { return m_dropped; }
    // Mutlak satır: 0..scrollbackSize()-1 geçmiş, sonrası görünen ekran
    Line line(int absoluteLine) const;
    QString lineText(int absoluteLine) const;
    // İki konum arasındaki metin (seçim), satır sonlarındaki boşluklar atılır
    QString text(Position from, Position to) const;

    int cursorRow() const { return m_cursorRow; }
    int cursorColumn() const { return m_cursorColumn; }
    bool cursorVisible() const { return m_cursorVisible; }
    // Kaçış dizisiyle (OSC 0/2) verilen başlık
    QString title() const { return m_title; }
    // Tam ekran uygulamalar (vi, less) alternatif ekranı kullanır, o sırada geçmiş büyümez
    bool alternateScreen() const { return m_alternate; }

    // Kabuğa gidecek baytlar (tuşlar, terminalin cevapları)
    std::function<void(const char *, size_t)> output;
    // Ekran değişti (çizim gerekiyor)
    std::function<void()> changed;
    std::function<void()> titleChanged;
    std::function<void()> bell;

private:
    static int damage(VTermRect rect, void *user);
    static int moveCursor(VTermPos pos, VTermPos oldpos, int visible, void *user);
    static int setTermProp(VTermProp prop, VTermValue *val, void *user);
    static int ring(void *user);
    static int pushLine(int cols, const VTermScreenCell *cells, void *user);
    static int popLine(int cols, VTermScreenCell *cells, void *user);
    static void outputCallback(const char *s, size_t len, void *user);

    Cell convert(const VTermScreenCell &cell) const;

    VTerm *m_vterm = nullptr;
    VTermScreen *m_screen = nullptr;
    int m_rows;
    int m_columns;
    int m_scrollbackLimit;
    int m_dropped = 0;
    std::deque<Line> m_scrollback;
    int m_cursorRow = 0;
    int m_cursorColumn = 0;
    bool m_cursorVisible = true;
    bool m_alternate = false;
    QString m_title;
    QByteArray m_titleBuffer;
};
