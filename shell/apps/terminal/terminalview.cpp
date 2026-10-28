#include "terminalview.h"

#include "ptyprocess.h"

#include <QClipboard>
#include <QCursor>
#include <QDir>
#include <QFontMetricsF>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QPainter>
#include <QSysInfo>
#include <QTimer>

#include <cmath>
#include <pwd.h>
#include <unistd.h>

namespace {

// Metnin kenarlardan uzaklığı (Figma: 16 yatay, 12 üst)
constexpr qreal paddingX = 16;
constexpr qreal paddingY = 12;

bool isWordCharacter(const QString &text)
{
    return !text.isEmpty() &&
           (text.at(0).isLetterOrNumber() || QStringLiteral("_-./~").contains(text));
}

} // namespace

TerminalView::TerminalView(QQuickItem *parent)
    : QQuickPaintedItem(parent)
{
    setAcceptedMouseButtons(Qt::LeftButton | Qt::MiddleButton);
    setFlag(ItemAcceptsInputMethod, false);
    setFlag(ItemIsFocusScope, true);
    setActiveFocusOnTab(false);
    setCursor(Qt::IBeamCursor);
    setOpaquePainting(true);
    m_font.setFamily(QStringLiteral("JetBrains Mono"));
    m_font.setPixelSize(13);
    m_font.setStyleHint(QFont::Monospace);
    updateMetrics();

    m_titleTimer = new QTimer(this);
    m_titleTimer->setSingleShot(true);
    m_titleTimer->setInterval(200);
    connect(m_titleTimer, &QTimer::timeout, this, &TerminalView::updateTitle);
    connect(this, &TerminalView::colorsChanged, this, [this] { update(); });
}

TerminalView::~TerminalView() = default;

void TerminalView::setFontFamily(const QString &family)
{
    if (family == m_font.family())
        return;
    m_font.setFamily(family);
    updateMetrics();
}

void TerminalView::setFontSize(int size)
{
    size = std::clamp(size, 8, 32);
    if (size == m_font.pixelSize())
        return;
    m_font.setPixelSize(size);
    updateMetrics();
}

void TerminalView::updateMetrics()
{
    m_boldFont = m_font;
    m_boldFont.setBold(true);
    const QFontMetricsF metrics(m_font);
    m_cellWidth = metrics.horizontalAdvance(QLatin1Char('M'));
    // Satır aralığı Figma'daki gibi geniş: 13 px yazıda 22 px
    m_cellHeight = std::ceil(m_font.pixelSize() * 1.7);
    m_ascent = std::round((m_cellHeight - metrics.height()) / 2 + metrics.ascent());
    emit fontChanged();
    updateSize();
    update();
}

void TerminalView::componentComplete()
{
    QQuickPaintedItem::componentComplete();
    const int columns = std::max(10, int((width() - 2 * paddingX) / m_cellWidth));
    const int rows = std::max(3, int((height() - 2 * paddingY) / m_cellHeight));
    m_screen = std::make_unique<TerminalScreen>(rows, columns);
    m_screen->changed = [this] { update(); };
    m_screen->titleChanged = [this] { updateTitle(); };

    m_pty = new PtyProcess(this);
    m_screen->output = [this](const char *data, size_t size) { m_pty->write(data, size); };
    connect(m_pty, &PtyProcess::received, this, [this](const QByteArray &data) {
        m_screen->write(data);
        // Yeni çıktı gelince en alta dön (çalışan bir komutu izlerken)
        if (m_scroll != 0 && !m_selecting)
            m_scroll = 0;
        if (!m_titleTimer->isActive())
            m_titleTimer->start();
    });
    connect(m_pty, &PtyProcess::finished, this, &TerminalView::finished);
    if (!m_pty->start(rows, columns))
        feed(tr("Kabuk başlatılamadı.\r\n"));
    updateTitle();
    emit sizeChanged();
}

void TerminalView::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickPaintedItem::geometryChange(newGeometry, oldGeometry);
    if (newGeometry.size() != oldGeometry.size())
        updateSize();
}

void TerminalView::updateSize()
{
    if (!m_screen || width() <= 0 || height() <= 0)
        return;
    const int columns = std::max(10, int((width() - 2 * paddingX) / m_cellWidth));
    const int rows = std::max(3, int((height() - 2 * paddingY) / m_cellHeight));
    if (columns == m_screen->columns() && rows == m_screen->rows())
        return;
    m_screen->resize(rows, columns);
    if (m_pty)
        m_pty->resize(rows, columns);
    emit sizeChanged();
}

void TerminalView::updateTitle()
{
    QString title = m_screen ? m_screen->title() : QString();
    if (title.isEmpty()) {
        const QString home = QDir::homePath();
        const QString directory = m_pty ? m_pty->workingDirectory() : QString();
        if (directory.isEmpty() || directory == home) {
            const passwd *account = getpwuid(getuid());
            const QString user = account ? QString::fromLocal8Bit(account->pw_name) : QString();
            QString host = QSysInfo::machineHostName();
            host = host.section(QLatin1Char('.'), 0, 0);
            title = QStringLiteral("%1@%2: ~").arg(user, host);
        } else {
            title = directory == QLatin1String("/") ? directory : QDir(directory).dirName();
        }
    }
    if (title != m_title) {
        m_title = title;
        emit titleChanged();
    }
}

void TerminalView::feed(const QString &text)
{
    if (m_screen)
        m_screen->write(text.toUtf8());
}

QColor TerminalView::color(const QColor &rgb, int index, bool foreground, bool bold) const
{
    if (index >= 0 && index < m_palette.size())
        return m_palette.at(bold && foreground && index < 8 ? index + 8 : index);
    if (rgb.isValid())
        return rgb;
    return foreground ? m_foreground : m_background;
}

void TerminalView::paint(QPainter *painter)
{
    painter->fillRect(QRectF(0, 0, width(), height()), m_background);
    if (!m_screen)
        return;

    const int rows = m_screen->rows();
    const int first = m_screen->scrollbackSize() - m_scroll;
    const int dropped = m_screen->droppedLines();
    TerminalScreen::Position selectionStart = m_selectionStart;
    TerminalScreen::Position selectionEnd = m_selectionEnd;
    if (selectionEnd < selectionStart)
        std::swap(selectionStart, selectionEnd);
    const bool selection = selectionStart != selectionEnd;

    for (int row = 0; row < rows; ++row) {
        const int absolute = first + row;
        const TerminalScreen::Line line = m_screen->line(absolute);
        const qreal y = paddingY + row * m_cellHeight;

        // Önce arka planlar, sonra metin: geniş karakterler komşu hücreye taşabilir
        struct Run
        {
            int column;
            QString text;
            QColor color;
            bool bold, italic, underline;
        };
        std::vector<Run> runs;
        for (int column = 0; column < int(line.size()); ++column) {
            const TerminalScreen::Cell &cell = line[size_t(column)];
            QColor foreground = color(cell.foreground, cell.foregroundIndex, true, cell.bold);
            QColor background = color(cell.background, cell.backgroundIndex, false, false);
            if (cell.reverse)
                std::swap(foreground, background);
            const TerminalScreen::Position here{absolute + dropped, column};
            const bool selected = selection && !(here < selectionStart) && here < selectionEnd;
            const qreal x = paddingX + column * m_cellWidth;
            if (background != m_background)
                painter->fillRect(QRectF(x, y, m_cellWidth * cell.width, m_cellHeight), background);
            if (selected)
                painter->fillRect(QRectF(x, y, m_cellWidth * cell.width, m_cellHeight),
                                  m_selectionColor);

            if (cell.text.isEmpty() || cell.text == QLatin1String(" "))
                continue;
            // Aynı biçimdeki tek genişlikli karakterler tek seferde çizilir
            const bool simple =
                cell.width == 1 && cell.text.size() == 1 && cell.text.at(0).unicode() < 0x2500;
            if (simple && !runs.empty()) {
                Run &last = runs.back();
                if (last.column + last.text.size() == column && last.color == foreground &&
                    last.bold == cell.bold && last.italic == cell.italic &&
                    last.underline == cell.underline && last.text.back().unicode() < 0x2500) {
                    last.text += cell.text;
                    continue;
                }
            }
            runs.push_back({column, cell.text, foreground, cell.bold, cell.italic, cell.underline});
        }
        for (const Run &run : runs) {
            QFont font = run.bold ? m_boldFont : m_font;
            font.setItalic(run.italic);
            font.setUnderline(run.underline);
            painter->setFont(font);
            painter->setPen(run.color);
            painter->drawText(QPointF(paddingX + run.column * m_cellWidth, y + m_ascent), run.text);
        }
    }

    // İmleç: odaktayken dolu blok, değilken çerçeve
    if (m_scroll == 0 && m_screen->cursorVisible()) {
        const QRectF cursor(paddingX + m_screen->cursorColumn() * m_cellWidth,
                            paddingY + m_screen->cursorRow() * m_cellHeight + 1.5, m_cellWidth,
                            m_cellHeight - 3);
        if (hasActiveFocus()) {
            painter->fillRect(cursor, m_cursorColor);
            const TerminalScreen::Line line =
                m_screen->line(m_screen->scrollbackSize() + m_screen->cursorRow());
            if (m_screen->cursorColumn() < int(line.size())) {
                const TerminalScreen::Cell &cell = line[size_t(m_screen->cursorColumn())];
                if (!cell.text.isEmpty()) {
                    painter->setFont(cell.bold ? m_boldFont : m_font);
                    painter->setPen(m_background);
                    painter->drawText(
                        QPointF(cursor.x(),
                                paddingY + m_screen->cursorRow() * m_cellHeight + m_ascent),
                        cell.text);
                }
            }
        } else {
            painter->setPen(QPen(m_cursorColor, 1));
            painter->setBrush(Qt::NoBrush);
            painter->drawRect(cursor.adjusted(0.5, 0.5, -0.5, -0.5));
        }
    }
}

TerminalScreen::Position TerminalView::cellAt(QPointF point) const
{
    const int row = std::clamp(int((point.y() - paddingY) / m_cellHeight), 0, m_screen->rows() - 1);
    const int column =
        std::clamp(int(std::round((point.x() - paddingX) / m_cellWidth)), 0, m_screen->columns());
    return {m_screen->scrollbackSize() - m_scroll + row + m_screen->droppedLines(), column};
}

void TerminalView::scrollBy(int lines)
{
    if (!m_screen)
        return;
    const int scroll = std::clamp(m_scroll + lines, 0, m_screen->scrollbackSize());
    if (scroll != m_scroll) {
        m_scroll = scroll;
        update();
    }
}

void TerminalView::copy()
{
    if (!m_screen || !hasSelection())
        return;
    const int dropped = m_screen->droppedLines();
    QGuiApplication::clipboard()->setText(
        m_screen->text({m_selectionStart.line - dropped, m_selectionStart.column},
                       {m_selectionEnd.line - dropped, m_selectionEnd.column}));
}

void TerminalView::paste()
{
    const QString text = QGuiApplication::clipboard()->text();
    if (m_screen && !text.isEmpty()) {
        m_scroll = 0;
        m_screen->paste(text);
    }
}

bool TerminalView::event(QEvent *event)
{
    // Ctrl+Shift kısayolları (yeni sekme, kopyala...) ve yazı boyutu pencerenin kısayollarına
    // kalır, diğer tuşlar (Ctrl+C, Ctrl+W...) kabuğa gider.
    if (event->type() == QEvent::ShortcutOverride) {
        auto *key = static_cast<QKeyEvent *>(event);
        const Qt::KeyboardModifiers modifiers = key->modifiers() & ~Qt::KeypadModifier;
        const bool windowShortcut = modifiers == (Qt::ControlModifier | Qt::ShiftModifier) ||
                                    (modifiers.testFlag(Qt::ControlModifier) &&
                                     (key->key() == Qt::Key_Plus || key->key() == Qt::Key_Equal ||
                                      key->key() == Qt::Key_Minus || key->key() == Qt::Key_0));
        if (!windowShortcut) {
            event->accept();
            return true;
        }
    }
    return QQuickPaintedItem::event(event);
}

void TerminalView::keyPressEvent(QKeyEvent *event)
{
    if (!m_screen)
        return;
    if (event->modifiers() == Qt::ShiftModifier &&
        (event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown)) {
        scrollBy((event->key() == Qt::Key_PageUp ? 1 : -1) * std::max(1, m_screen->rows() - 1));
        return;
    }
    if (event->key() == Qt::Key_Shift || event->key() == Qt::Key_Control ||
        event->key() == Qt::Key_Alt || event->key() == Qt::Key_Meta ||
        event->key() == Qt::Key_CapsLock)
        return;
    m_scroll = 0;
    if (hasSelection()) {
        m_selectionStart = m_selectionEnd;
        emit selectionChanged();
    }
    m_screen->key(event->key(), event->modifiers(), event->text());
    update();
}

void TerminalView::mousePressEvent(QMouseEvent *event)
{
    forceActiveFocus(Qt::MouseFocusReason);
    if (!m_screen)
        return;
    if (event->button() == Qt::MiddleButton) {
        paste();
        return;
    }
    m_selecting = true;
    m_selectionStart = m_selectionEnd = cellAt(event->position());
    emit selectionChanged();
    update();
}

void TerminalView::mouseMoveEvent(QMouseEvent *event)
{
    if (!m_selecting)
        return;
    // Pencerenin dışına sürükleyince kaydır
    if (event->position().y() < 0)
        scrollBy(1);
    else if (event->position().y() > height())
        scrollBy(-1);
    m_selectionEnd = cellAt(event->position());
    emit selectionChanged();
    update();
}

void TerminalView::mouseReleaseEvent(QMouseEvent *)
{
    m_selecting = false;
}

// Çift tıklama kelimeyi seçer
void TerminalView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (!m_screen)
        return;
    const TerminalScreen::Position at = cellAt(event->position() - QPointF(m_cellWidth / 2, 0));
    const TerminalScreen::Line line = m_screen->line(at.line - m_screen->droppedLines());
    if (at.column >= int(line.size()) || !isWordCharacter(line[size_t(at.column)].text))
        return;
    int start = at.column;
    int end = at.column + 1;
    while (start > 0 && isWordCharacter(line[size_t(start - 1)].text))
        --start;
    while (end < int(line.size()) && isWordCharacter(line[size_t(end)].text))
        ++end;
    m_selectionStart = {at.line, start};
    m_selectionEnd = {at.line, end};
    m_selecting = false;
    emit selectionChanged();
    update();
}

void TerminalView::wheelEvent(QWheelEvent *event)
{
    if (!m_screen)
        return;
    const int lines = event->angleDelta().y() / 40;
    if (lines == 0)
        return;
    // Tam ekran uygulamalarda (less, vi) tekerlek ok tuşu olur
    if (m_screen->alternateScreen()) {
        for (int i = 0; i < std::abs(lines); ++i)
            m_screen->key(lines > 0 ? Qt::Key_Up : Qt::Key_Down, Qt::NoModifier, {});
        return;
    }
    scrollBy(lines);
}
