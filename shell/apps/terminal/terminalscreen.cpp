#include "terminalscreen.h"

#include <vterm.h>

namespace {

VTermModifier modifiersFor(Qt::KeyboardModifiers modifiers)
{
    int result = VTERM_MOD_NONE;
    if (modifiers & Qt::ShiftModifier)
        result |= VTERM_MOD_SHIFT;
    if (modifiers & Qt::AltModifier)
        result |= VTERM_MOD_ALT;
    if (modifiers & Qt::ControlModifier)
        result |= VTERM_MOD_CTRL;
    return VTermModifier(result);
}

VTermKey keyFor(int qtKey)
{
    switch (qtKey) {
    case Qt::Key_Return:
        return VTERM_KEY_ENTER;
    case Qt::Key_Enter:
        return VTERM_KEY_KP_ENTER;
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
        return VTERM_KEY_TAB;
    case Qt::Key_Backspace:
        return VTERM_KEY_BACKSPACE;
    case Qt::Key_Escape:
        return VTERM_KEY_ESCAPE;
    case Qt::Key_Up:
        return VTERM_KEY_UP;
    case Qt::Key_Down:
        return VTERM_KEY_DOWN;
    case Qt::Key_Left:
        return VTERM_KEY_LEFT;
    case Qt::Key_Right:
        return VTERM_KEY_RIGHT;
    case Qt::Key_Insert:
        return VTERM_KEY_INS;
    case Qt::Key_Delete:
        return VTERM_KEY_DEL;
    case Qt::Key_Home:
        return VTERM_KEY_HOME;
    case Qt::Key_End:
        return VTERM_KEY_END;
    case Qt::Key_PageUp:
        return VTERM_KEY_PAGEUP;
    case Qt::Key_PageDown:
        return VTERM_KEY_PAGEDOWN;
    default:
        if (qtKey >= Qt::Key_F1 && qtKey <= Qt::Key_F12)
            return VTermKey(VTERM_KEY_FUNCTION(qtKey - Qt::Key_F1 + 1));
        return VTERM_KEY_NONE;
    }
}

QColor rgb(VTermScreen *screen, VTermColor color)
{
    vterm_screen_convert_color_to_rgb(screen, &color);
    return QColor(color.rgb.red, color.rgb.green, color.rgb.blue);
}

} // namespace

TerminalScreen::TerminalScreen(int rows, int columns, int scrollbackLimit)
    : m_rows(rows)
    , m_columns(columns)
    , m_scrollbackLimit(scrollbackLimit)
{
    m_vterm = vterm_new(rows, columns);
    vterm_set_utf8(m_vterm, 1);
    vterm_output_set_callback(m_vterm, &TerminalScreen::outputCallback, this);

    m_screen = vterm_obtain_screen(m_vterm);
    static const VTermScreenCallbacks callbacks = {
        .damage = &TerminalScreen::damage,
        .moverect = nullptr,
        .movecursor = &TerminalScreen::moveCursor,
        .settermprop = &TerminalScreen::setTermProp,
        .bell = &TerminalScreen::ring,
        .resize = nullptr,
        .sb_pushline = &TerminalScreen::pushLine,
        .sb_popline = &TerminalScreen::popLine,
        .sb_clear = nullptr,
    };
    vterm_screen_set_callbacks(m_screen, &callbacks, this);
    vterm_screen_set_damage_merge(m_screen, VTERM_DAMAGE_SCROLL);
    vterm_screen_enable_altscreen(m_screen, 1);
    vterm_screen_reset(m_screen, 1);
}

TerminalScreen::~TerminalScreen()
{
    vterm_free(m_vterm);
}

void TerminalScreen::write(const char *data, size_t size)
{
    vterm_input_write(m_vterm, data, size);
    vterm_screen_flush_damage(m_screen);
}

void TerminalScreen::resize(int rows, int columns)
{
    if (rows == m_rows && columns == m_columns)
        return;
    m_rows = rows;
    m_columns = columns;
    vterm_set_size(m_vterm, rows, columns);
    vterm_screen_flush_damage(m_screen);
}

void TerminalScreen::key(int qtKey, Qt::KeyboardModifiers modifiers, const QString &text)
{
    const VTermModifier mod = modifiersFor(modifiers);
    const VTermKey special = keyFor(qtKey);
    if (special != VTERM_KEY_NONE) {
        vterm_keyboard_key(m_vterm, special, mod);
        return;
    }
    // Ctrl+harf: libvterm kontrol karakterine çevirir (Ctrl+C -> 0x03)
    if ((modifiers & Qt::ControlModifier) && qtKey >= Qt::Key_A && qtKey <= Qt::Key_Z) {
        vterm_keyboard_unichar(m_vterm, uint32_t('a' + (qtKey - Qt::Key_A)), mod);
        return;
    }
    // Yazılan metinde Shift zaten büyük harfe yansımıştır
    const VTermModifier textMod = VTermModifier(mod & ~VTERM_MOD_SHIFT);
    for (const char32_t c : text.toUcs4()) {
        if (c >= 0x20 && c != 0x7f)
            vterm_keyboard_unichar(m_vterm, uint32_t(c), textMod);
    }
}

void TerminalScreen::paste(const QString &text)
{
    QString normalized = text;
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\r"));
    normalized.replace(QLatin1Char('\n'), QLatin1Char('\r'));
    const QByteArray bytes = normalized.toUtf8();
    vterm_keyboard_start_paste(m_vterm);
    if (output)
        output(bytes.constData(), size_t(bytes.size()));
    vterm_keyboard_end_paste(m_vterm);
}

TerminalScreen::Line TerminalScreen::line(int absoluteLine) const
{
    if (absoluteLine < 0)
        return {};
    if (absoluteLine < scrollbackSize())
        return m_scrollback[size_t(absoluteLine)];
    const int row = absoluteLine - scrollbackSize();
    if (row >= m_rows)
        return {};
    Line result;
    result.reserve(size_t(m_columns));
    for (int column = 0; column < m_columns; ++column) {
        VTermScreenCell cell;
        vterm_screen_get_cell(m_screen, VTermPos{row, column}, &cell);
        result.push_back(convert(cell));
    }
    return result;
}

QString TerminalScreen::lineText(int absoluteLine) const
{
    QString text;
    for (const Cell &cell : line(absoluteLine))
        text += cell.text.isEmpty() && cell.width == 1 ? QStringLiteral(" ") : cell.text;
    while (text.endsWith(QLatin1Char(' ')))
        text.chop(1);
    return text;
}

QString TerminalScreen::text(Position from, Position to) const
{
    if (to < from)
        std::swap(from, to);
    QStringList lines;
    for (int number = from.line; number <= to.line; ++number) {
        const Line cells = line(number);
        const int first = number == from.line ? from.column : 0;
        const int last = number == to.line ? to.column : int(cells.size());
        QString text;
        for (int column = first; column < last && column < int(cells.size()); ++column) {
            const Cell &cell = cells[size_t(column)];
            text += cell.text.isEmpty() && cell.width == 1 ? QStringLiteral(" ") : cell.text;
        }
        while (text.endsWith(QLatin1Char(' ')))
            text.chop(1);
        lines.append(text);
    }
    return lines.join(QLatin1Char('\n'));
}

TerminalScreen::Cell TerminalScreen::convert(const VTermScreenCell &cell) const
{
    Cell result;
    // Geniş karakterin ikinci hücresi libvterm'de (uint32_t)-1 ile işaretlidir
    if (cell.chars[0] == uint32_t(-1)) {
        result.width = 0;
        return result;
    }
    for (int i = 0; i < VTERM_MAX_CHARS_PER_CELL && cell.chars[i] != 0; ++i)
        result.text += QString::fromUcs4(reinterpret_cast<const char32_t *>(&cell.chars[i]), 1);
    result.width = cell.width;
    result.bold = cell.attrs.bold;
    result.italic = cell.attrs.italic;
    result.underline = cell.attrs.underline != 0;
    result.reverse = cell.attrs.reverse;

    const auto color = [this](const VTermColor &color, QColor &rgbColor, int &index,
                              bool isDefault) {
        if (isDefault)
            return;
        if (VTERM_COLOR_IS_INDEXED(&color) && color.indexed.idx < 16)
            index = color.indexed.idx;
        else
            rgbColor = rgb(m_screen, color);
    };
    color(cell.fg, result.foreground, result.foregroundIndex, VTERM_COLOR_IS_DEFAULT_FG(&cell.fg));
    color(cell.bg, result.background, result.backgroundIndex, VTERM_COLOR_IS_DEFAULT_BG(&cell.bg));
    return result;
}

int TerminalScreen::damage(VTermRect, void *user)
{
    auto *self = static_cast<TerminalScreen *>(user);
    if (self->changed)
        self->changed();
    return 1;
}

int TerminalScreen::moveCursor(VTermPos pos, VTermPos, int visible, void *user)
{
    auto *self = static_cast<TerminalScreen *>(user);
    self->m_cursorRow = pos.row;
    self->m_cursorColumn = pos.col;
    self->m_cursorVisible = visible;
    if (self->changed)
        self->changed();
    return 1;
}

int TerminalScreen::setTermProp(VTermProp prop, VTermValue *val, void *user)
{
    auto *self = static_cast<TerminalScreen *>(user);
    switch (prop) {
    case VTERM_PROP_CURSORVISIBLE:
        self->m_cursorVisible = val->boolean;
        break;
    case VTERM_PROP_ALTSCREEN:
        self->m_alternate = val->boolean;
        break;
    case VTERM_PROP_TITLE:
        // Başlık parça parça gelebilir
        if (val->string.initial)
            self->m_titleBuffer.clear();
        self->m_titleBuffer.append(val->string.str, qsizetype(val->string.len));
        if (val->string.final) {
            self->m_title = QString::fromUtf8(self->m_titleBuffer);
            if (self->titleChanged)
                self->titleChanged();
        }
        break;
    default:
        break;
    }
    if (self->changed)
        self->changed();
    return 1;
}

int TerminalScreen::ring(void *user)
{
    auto *self = static_cast<TerminalScreen *>(user);
    if (self->bell)
        self->bell();
    return 1;
}

int TerminalScreen::pushLine(int cols, const VTermScreenCell *cells, void *user)
{
    auto *self = static_cast<TerminalScreen *>(user);
    Line line;
    line.reserve(size_t(cols));
    for (int i = 0; i < cols; ++i)
        line.push_back(self->convert(cells[i]));
    self->m_scrollback.push_back(std::move(line));
    if (int(self->m_scrollback.size()) > self->m_scrollbackLimit) {
        self->m_scrollback.pop_front();
        ++self->m_dropped;
    }
    return 1;
}

// Pencere büyüyünce libvterm geçmişten satır geri ister
int TerminalScreen::popLine(int cols, VTermScreenCell *cells, void *user)
{
    auto *self = static_cast<TerminalScreen *>(user);
    if (self->m_scrollback.empty())
        return 0;
    const Line line = std::move(self->m_scrollback.back());
    self->m_scrollback.pop_back();
    for (int i = 0; i < cols; ++i) {
        VTermScreenCell &cell = cells[i];
        cell = {};
        cell.width = 1;
        vterm_color_indexed(&cell.fg, 7);
        cell.fg.type |= VTERM_COLOR_DEFAULT_FG;
        vterm_color_indexed(&cell.bg, 0);
        cell.bg.type |= VTERM_COLOR_DEFAULT_BG;
        if (i < int(line.size())) {
            const Cell &source = line[size_t(i)];
            if (source.width == 0) {
                cell.chars[0] = uint32_t(-1);
                continue;
            }
            const std::u32string chars = source.text.toStdU32String();
            for (size_t c = 0; c < chars.size() && c < VTERM_MAX_CHARS_PER_CELL; ++c)
                cell.chars[c] = uint32_t(chars[c]);
            cell.width = char(source.width);
            cell.attrs.bold = source.bold;
            cell.attrs.italic = source.italic;
            cell.attrs.underline = source.underline;
            cell.attrs.reverse = source.reverse;
            if (source.foregroundIndex >= 0)
                vterm_color_indexed(&cell.fg, uint8_t(source.foregroundIndex));
            else if (source.foreground.isValid())
                vterm_color_rgb(&cell.fg, uint8_t(source.foreground.red()),
                                uint8_t(source.foreground.green()),
                                uint8_t(source.foreground.blue()));
            if (source.backgroundIndex >= 0)
                vterm_color_indexed(&cell.bg, uint8_t(source.backgroundIndex));
            else if (source.background.isValid())
                vterm_color_rgb(&cell.bg, uint8_t(source.background.red()),
                                uint8_t(source.background.green()),
                                uint8_t(source.background.blue()));
        }
    }
    return 1;
}

void TerminalScreen::outputCallback(const char *s, size_t len, void *user)
{
    auto *self = static_cast<TerminalScreen *>(user);
    if (self->output)
        self->output(s, len);
}
